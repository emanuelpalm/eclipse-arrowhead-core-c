// SPDX-License-Identifier: EPL-2.0

#ifndef AH_LOOP_H
#define AH_LOOP_H

#include "ahi/loop.h"
#include "alloc.h"
#include "def.h"
#include "time.h"

#include <ahp/loop.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @file
 * Event scheduling.
 *
 * This file exposes the <em>event loop</em>, which is a data structure used to
 * subscribe to and schedule platform events. Examples of events can be a
 * received network message, a file finished being copied, or a certain task
 * having been executed until completion.
 *
 * While the API provided here only allows for operations directly on the event
 * loops themselves, other features of this library make use of event loops
 * internally in order to handle event scheduling, such as those in @c task.h,
 * @c net/tcp.h and @c net/udp.h.
 *
 * The event loop data structure is typically implemented on top of an API that
 * is provided by the platform for which this library is compiled. The
 * following table show what platform APIs are used on the respective platforms
 * supported by this library:
 *
 * <table>
 *   <caption id="event-loop-implementations">Event loop implementations</caption>
 *   <tr>
 *     <th>Platform
 *     <th>API
 *   <tr>
 *     <td>Darwin
 *     <td><a href="https://developer.apple.com/library/archive/documentation/Darwin/Conceptual/FSEvents_ProgGuide/KernelQueues/KernelQueues.html">Kernel Queues (KQueue)</a>
 *   <tr>
 *     <td>Linux
 *     <td><a href="https://unixism.net/loti/what_is_io_uring.html">io_uring</a>
 *   <tr>
 *     <td>Win32
 *     <td><a href="https://docs.microsoft.com/en-us/windows/win32/fileio/i-o-completion-ports">I/O Completion Ports (IOCP)</a>
 * </table>
 *
 * @warning Event loops are @a not thread safe, and, as a consequence, neither
 *          are any functions that depend on event loops throughout this
 *          library.
 */

/**
 * An event loop.
 */
struct ah_loop {
    ah_slab_t _evt_slab;
    ah_time_t _now;
    ahp_loop_t _p;
    int _state;
};

/**
 * Initializes event loop @a l.
 *
 * @param[out] l Pointer to event loop.
 *
 * @retval AH_OK           if @a l was successfully initialized.
 * @retval AH_EINVAL       if @a l is @c NULL.
 * @retval AH_EMFILE       [Darwin, Linux] if the per-process file descriptor
 *                         table is full.
 * @retval AH_ENFILE       [Darwin, Linux] if the platform file table is full.
 * @retval AH_ENOMEM       [Darwin, Linux] if failed to allocate kernel memory
 *                         for event queue.
 * @retval AH_EOVERFLOW    [Linux] if more than 32-bits of heap memory was
 *                         requested on a 32-bit system.
 * @retval AH_EPERM        [Linux] if permission was denied to set up required
 *                         kernel resource.
 * @retval AH_EPROCLIM_W   [Win32] if the Windows task limit was reached.
 * @retval AH_ESYSNOTREADY [Win32] if the network subsystem is not ready.
 *
 * @warning No other functions operating on @a l are safe to call until
 *          after this function has returned successfully, unless something
 *          else is stated in their respective documentations.
 */
ah_err_t ah_loop_init(ah_loop_t* l);

/**
 * Checks if event loop @a l is currently running.
 *
 * A loop is running if and only if (1) @c ah_loop_run() is currently being
 * invoked with a pointer to that loop as argument, and (2) no call has been
 * made to @c ah_loop_stop() or @c ah_loop_term() with that loop since
 * @c ah_loop_run() was first invoked.
 *
 * @param[in] l Pointer to event loop.
 *
 * @return @c true only if @a l is currently running. @c false otherwise.
 *
 * @note The function returns @c false if @a l is @c NULL.
 */
ah_inline bool ah_loop_is_running(const ah_loop_t* l)
{
    return l != NULL && l->_state == AHI_LOOP_STATE_RUNNING;
}

/**
 * Checks if event loop  @a l is currently being or has been terminated.
 *
 * A loop is being or has been terminated if @c ah_loop_term() has been invoked
 * with a pointer to it.
 *
 * @param[in] l Pointer to event loop.
 *
 * @return @c true only if @a l is currently being or has been terminated.
 *         @c false otherwise.
 *
 * @note The function returns @c false if @a l is @c NULL.
 *
 * @warning This function is only safe to use if (1) the memory of @a l is
 *         zeroed, (2) @a l has been initialized using @c ah_loop_init() and
 *         is currently in a non-terminated state, or (3) @a l has been
 *         terminated using @c ah_loop_term().
 */
ah_inline bool ah_loop_is_term(const ah_loop_t* l)
{
    return l != NULL && l->_state >= AHI_LOOP_STATE_TERMINATING;
}

/**
 * Gets current time, as kept track of by event loop @a l.
 *
 * This function exists solely as a way of getting a relatively accurate
 * estimate of the current time without having to use @c ah_time_now(), which
 * uses a relatively costly system call on some platforms.
 *
 * @param[in] l Pointer to event loop.
 *
 * @return Time at which @a l last updated its internal clock.
 *
 * @note The function returns @c AH_TIME_ZERO if @a l is @c NULL.
 */
ah_inline ah_time_t ah_loop_now(const ah_loop_t* l)
{
    return l == NULL ? AH_TIME_ZERO : l->_now;
}

/**
 * Runs event loop @a l at least until time @a t, making it await and handle
 * event completions for that duration.
 *
 * If @a time is @c NULL, the function will block indefinitely. If no events
 * were registered prior to such a call, no opportunity will be given to call
 * @c ah_loop_stop() or @c ah_loop_term() from the blocked thread. As event
 * loops are not thread safe by design, this leaves no safe way of ever stopping
 * the loop other than by terminating or interrupting the application from
 * another thread or process.
 *
 * The operation blocks until some time after (1) ah_loop_stop() or
 * ah_loop_term() is called with @a l as argument, or (2) @a t is passed.
 *
 * @param[in] l Pointer to event loop.
 * @param[in] t Point after which @a l execution is to stop, or @c NULL.
 *
 * @retval AH_OK     if @a l ran until @a t expired, it was stopped or it was
 *                   terminated.
 * @retval AH_ERANGE if @a time is too far into the future for it to be
 *                   representable by the kernel event queue system.
 * @retval AH_EINVAL if @a l is @c NULL.
 * @retval AH_ESTATE if @a l is already running or has been terminated.
 * @retval AH_EACCES [Darwin] if the process lacks permission to register KQueue
 *                   filter.
 * @retval AH_EINTR  [Darwin, Linux] if the process was interrupted by a signal.
 * @retval AH_ENOMEM [Darwin, Linux] if failed to submit pending events due to
 *                   no memory being available to the kernel.
 */
ah_err_t ah_loop_run_until(ah_loop_t* l, ah_time_t* t);

/**
 * Stops event loop @a l, preventing it from processing any further events.
 *
 * @param[in] l Pointer to event loop.
 *
 * @retval AH_OK     if @a l was stopped.
 * @retval AH_EINVAL if @a l is @c NULL.
 * @retval AH_ESTATE if @a l is not running.
 */
ah_inline ah_err_t ah_loop_stop(ah_loop_t* l)
{
    if (l == NULL) {
        return AH_EINVAL;
    }
    if (l->_state != AHI_LOOP_STATE_RUNNING) {
        return AH_ESTATE;
    }
    l->_state = AHI_LOOP_STATE_STOPPING;
    return AH_OK;
}

/**
 * Terminates @a l, cancelling all of its pending events and releases all of
 * its resources.
 *
 * All pending events of @a l will be invoked with @c AH_ECANCELED before
 * termination completes.
 *
 * If this function is called from an event handler while @c ah_loop_run() or
 * @c ah_loop_run_until() is executing on the same @a l, termination is
 * scheduled before either of the mentioned functions return. If @a l is not
 * currently running, the termination procedure is executed before this
 * function returns.
 *
 * @param[in] l Pointer to event loop.
 *
 * @retval AH_OK     if @a l was terminated or is scheduled for termination.
 * @retval AH_EINVAL if @a l is @c NULL.
 * @retval AH_ESTATE if @a l is already terminating or terminated.
 */
ah_err_t ah_loop_term(ah_loop_t* l);

#endif
