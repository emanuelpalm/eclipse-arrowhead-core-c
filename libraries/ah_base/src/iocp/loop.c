// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0.
//
// SPDX-License-Identifier: EPL-2.0

#include "ah/loop.h"

#include "ah/assert.h"
#include "ah/err.h"
#include "ah/task.h"

#include <stdlib.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32")

#define S_COMPLETION_ENTRY_STACK_HEIGHT 32u

static ah_err_t s_task_queue_init(struct ah_i_loop_task_queue* queue, ah_alloc_cb alloc_cb, size_t initial_capacity);
static ah_task_t* s_task_queue_dequeue_if_at_or_after(struct ah_i_loop_task_queue* queue, ah_time_t baseline);
static ah_time_t* s_task_queue_peek_at_baseline(struct ah_i_loop_task_queue* queue);
static void s_task_queue_heapify_down_from(struct ah_i_loop_task_queue* queue, const size_t index);
static void s_task_queue_term(struct ah_i_loop_task_queue* queue, ah_alloc_cb alloc_cb);

ah_extern ah_err_t ah_i_loop_init(ah_loop_t* loop, ah_loop_opts_t* opts)
{
    ah_assert_if_debug(loop != NULL);
    ah_assert_if_debug(opts != NULL);

    if (opts->alloc_cb == NULL) {
        opts->alloc_cb = realloc;
    }

    if (opts->capacity == 0u) {
        opts->capacity = 1024u;
    }

    ah_err_t err = s_task_queue_init(&loop->_task_queue, opts->alloc_cb, opts->capacity / 4u);
    if (err != AH_ENONE) {
        return err;
    }

    WSADATA wsa_data;
    int res = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (res != 0) {
        return res;
    }

    HANDLE iocp_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0u, 1u);
    if (iocp_handle == NULL) {
        return GetLastError();
    }

    loop->_iocp_handle = iocp_handle;

    return AH_ENONE;
}

ah_err_t ah_i_loop_poll_no_longer_than_until(ah_loop_t* loop, ah_time_t* time)
{
    ah_assert_if_debug(loop != NULL);

    ah_err_t err = ah_i_loop_get_pending_err(loop);
    if (err != AH_ENONE) {
        return err;
    }

    OVERLAPPED_ENTRY entries[S_COMPLETION_ENTRY_STACK_HEIGHT];
    ULONG num_entries_removed;

    do {
        loop->_now = ah_time_now();

        DWORD timeout_in_ms;
        {
            ah_time_t* poll_baseline = time;
            ah_time_t* task_baseline = s_task_queue_peek_at_baseline(&loop->_task_queue);

            ah_timediff_t poll_timeout;
            if (poll_baseline != NULL) {
                err = ah_time_diff(*poll_baseline, loop->_now, &poll_timeout);
                if (err != AH_ENONE) {
                    return err;
                }
            }
            else {
                poll_timeout = AH_TIMEDIFF_MAX;
            }

            ah_timediff_t task_timeout;
            if (task_baseline != NULL) {
                err = ah_time_diff(*task_baseline, loop->_now, &task_timeout);
                if (err != AH_ENONE) {
                    return err;
                }
            }
            else {
                task_timeout = AH_TIMEDIFF_MAX;
            }

            ah_timediff_t timeout = poll_timeout < task_timeout ? poll_timeout : task_timeout;

            if (timeout == AH_TIMEDIFF_MAX) {
                timeout_in_ms = INFINITE;
            }
            else if (timeout < 0u) {
                timeout_in_ms = 0;
            }
            else {
                ah_timediff_t tmp = timeout / 1000000;
                if (((uintmax_t) tmp) > ((uintmax_t) MAXDWORD)) {
                    timeout_in_ms = INFINITE - 1u;
                }
                else {
                    timeout_in_ms = (DWORD) tmp;
                }
            }
        }

        if (!GetQueuedCompletionStatusEx(loop->_iocp_handle, entries, S_COMPLETION_ENTRY_STACK_HEIGHT,
                &num_entries_removed, timeout_in_ms, false))
        {
            return GetLastError();
        }

        loop->_now = ah_time_now();

        for (ULONG i = 0u; i < num_entries_removed; i += 1u) {
            OVERLAPPED_ENTRY* overlapped_entry = &entries[i];
            ah_i_loop_evt_t* evt = CONTAINING_RECORD(overlapped_entry->lpOverlapped, ah_i_loop_evt_t, _overlapped);

            if (ah_likely(evt->_cb != NULL)) {
                evt->_cb(evt, overlapped_entry);
            }

            ah_i_loop_evt_dealloc(loop, evt);
        }

        for (;;) {
            ah_task_t* task = s_task_queue_dequeue_if_at_or_after(&loop->_task_queue, loop->_now);
            if (task == NULL) {
                break;
            }
            ah_i_task_execute_scheduled(task);
        }
    } while (num_entries_removed == S_COMPLETION_ENTRY_STACK_HEIGHT);

    return AH_ENONE;
}

ah_extern void ah_i_loop_term(ah_loop_t* loop)
{
    ah_assert_if_debug(loop != NULL);

    s_task_queue_term(&loop->_task_queue, loop->_alloc_cb);

    (void) CloseHandle(loop->_iocp_handle);
    (void) WSACleanup();
}

static ah_err_t s_task_queue_init(struct ah_i_loop_task_queue* queue, ah_alloc_cb alloc_cb, size_t initial_capacity)
{
    ah_assert_if_debug(queue != NULL);
    ah_assert_if_debug(alloc_cb != NULL);

    struct ah_i_loop_task_entry* entries;

    entries = ah_malloc_array(alloc_cb, initial_capacity, sizeof(struct ah_i_loop_task_entry));
    if (entries == NULL) {
        return AH_ENOMEM;
    }

    *queue = (struct ah_i_loop_task_queue) {
        ._capacity = initial_capacity,
        ._length = 0u,
        ._entries = entries,
    };

    return AH_ENONE;
}

static ah_task_t* s_task_queue_dequeue_if_at_or_after(struct ah_i_loop_task_queue* queue, ah_time_t baseline)
{
    ah_assert_if_debug(queue != NULL);

    if (queue->_length == 0u || ah_time_is_before(queue->_entries[0u]._baseline, baseline)) {
        return NULL;
    }
    queue->_length -= 1u;

    ah_task_t* task = queue->_entries[0u]._task;

    queue->_entries[0u] = queue->_entries[queue->_length];

    s_task_queue_heapify_down_from(queue, 0u);

    return task;
}

static ah_time_t* s_task_queue_peek_at_baseline(struct ah_i_loop_task_queue* queue)
{
    ah_assert_if_debug(queue != NULL);

    if (queue->_length == 0u) {
        return NULL;
    }

    return &queue->_entries[0u]._baseline;
}

static void s_task_queue_heapify_down_from(struct ah_i_loop_task_queue* queue, const size_t index)
{
    ah_assert_if_debug(queue != NULL);
    ah_assert_if_debug(((index * 2u) + 2u) <= queue->_capacity);

    struct ah_i_loop_task_entry* entries = queue->_entries;
    size_t length = queue->_length;

    size_t index_current = index;
    size_t index_min_child = index;

    while (index_current < length) {
        ah_time_t baseline = entries[index_current]._baseline;

        size_t index_left_child = (index_current * 2u) + 1u;
        if (index_left_child < length && ah_time_is_before(entries[index_left_child]._baseline, baseline)) {
            index_min_child = index_left_child;
        }

        size_t index_right_child = index_left_child + 1u;
        if (index_right_child < length && ah_time_is_before(entries[index_right_child]._baseline, baseline)) {
            index_min_child = index_right_child;
        }

        if (index_min_child == index_current) {
            break;
        }

        struct ah_i_loop_task_entry entry = entries[index_min_child];
        entries[index_min_child] = entries[index_current];
        entries[index_current] = entry;

        index_current = index_min_child;
    }
}

static void s_task_queue_term(struct ah_i_loop_task_queue* queue, ah_alloc_cb alloc_cb)
{
    ah_assert_if_debug(queue != NULL);
    ah_assert_if_debug(alloc_cb != NULL);

    ah_dealloc(alloc_cb, queue->_entries);

#ifndef NDEBUG
    *queue = (struct ah_i_loop_task_queue) { 0u };
#endif
}

ah_extern ah_err_t ah_i_loop_schedule_task(ah_loop_t* loop, ah_time_t baseline, ah_task_t* task)
{
    ah_assert_if_debug(loop != NULL);
    ah_assert_if_debug(task != NULL);

    struct ah_i_loop_task_queue* queue = &loop->_task_queue;

    if (queue->_length == queue->_capacity) {
        const size_t entry_size = sizeof(struct ah_i_loop_task_entry);
        struct ah_i_loop_task_entry* entries;

        entries = ah_realloc_array_larger(loop->_alloc_cb, queue->_entries, &queue->_capacity, entry_size);
        if (entries == NULL) {
            return AH_ENOMEM;
        }

        queue->_entries = entries;
    }

    size_t index = queue->_length;
    queue->_length += 1;

    while (index != 0u) {
        size_t index_parent = (index - 1u) / 2u;
        if (!ah_time_is_before(baseline, queue->_entries[index_parent]._baseline)) {
            break;
        }

        queue->_entries[index] = queue->_entries[index_parent];
        index = index_parent;
    }

    queue->_entries[index] = (struct ah_i_loop_task_entry) {
        ._baseline = baseline,
        ._task = task,
    };

    return AH_ENONE;
}

ah_extern bool ah_i_loop_try_cancel_task(ah_loop_t* loop, ah_task_t* task)
{
    ah_assert_if_debug(loop != NULL);
    ah_assert_if_debug(task != NULL);

    struct ah_i_loop_task_queue* queue = &loop->_task_queue;

    for (size_t index = 0u; index < queue->_length; index += 1u) {
        if (queue->_entries[index]._task != task) {
            continue;
        }

        queue->_entries[index] = queue->_entries[queue->_length];
        queue->_length -= 1u;

        s_task_queue_heapify_down_from(queue, index);

        return true;
    }

    return false;
}
