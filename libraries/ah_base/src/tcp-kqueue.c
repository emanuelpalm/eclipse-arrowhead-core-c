// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0.
//
// SPDX-License-Identifier: EPL-2.0

#include "ah/tcp.h"

#include "ah/assert.h"
#include "ah/err.h"
#include "ah/loop.h"

#include <sys/uio.h>

static void s_on_conn_connect(ah_i_loop_evt_t* evt, struct kevent* kev);
static void s_on_conn_read(ah_i_loop_evt_t* evt, struct kevent* kev);
static void s_on_conn_write(ah_i_loop_evt_t* evt, struct kevent* kev);

static void s_on_listener_accept(ah_i_loop_evt_t* evt, struct kevent* kev);

static ah_err_t s_prep_conn_write(ah_tcp_conn_t* conn);

ah_extern ah_err_t ah_tcp_conn_connect(ah_tcp_conn_t* conn, const ah_sockaddr_t* raddr)
{
    if (conn == NULL || raddr == NULL || !ah_sockaddr_is_ip(raddr)) {
        return AH_EINVAL;
    }
    if (conn->_state != AH_I_TCP_CONN_STATE_OPEN) {
        return AH_ESTATE;
    }

    ah_i_loop_evt_t* evt;
    struct kevent* kev;

    ah_err_t err = ah_i_loop_evt_alloc_with_kev(conn->_loop, &evt, &kev);
    if (err != AH_ENONE) {
        return err;
    }

    evt->_cb = s_on_conn_connect;
    evt->_subject = conn;

    conn->_state = AH_I_TCP_CONN_STATE_CONNECTING;

    EV_SET(kev, conn->_fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0u, 0u, evt);

    if (connect(conn->_fd, ah_i_sockaddr_const_into_bsd(raddr), ah_i_sockaddr_get_size(raddr)) != 0) {
        if (errno == EINPROGRESS) {
            return AH_ENONE;
        }
        kev->flags |= EV_ERROR;
        kev->data = errno;
    }

    s_on_conn_connect(evt, kev);

    return AH_ENONE;
}

static void s_on_conn_connect(ah_i_loop_evt_t* evt, struct kevent* kev)
{
    ah_assert_if_debug(evt != NULL);
    ah_assert_if_debug(kev != NULL);

    ah_tcp_conn_t* conn = evt->_subject;
    ah_assert_if_debug(conn != NULL);

    ah_err_t err;

    if (ah_unlikely((kev->flags & EV_ERROR) != 0)) {
        err = (ah_err_t) kev->data;
        conn->_state = AH_I_TCP_CONN_STATE_OPEN;
    }
    else if (ah_unlikely((kev->flags & EV_EOF) != 0)) {
        conn->_state = AH_I_TCP_CONN_STATE_OPEN;
        err = kev->fflags != 0 ? (ah_err_t) kev->fflags : AH_EEOF;
    }
    else {
        conn->_state = AH_I_TCP_CONN_STATE_CONNECTED;

        ah_tcp_shutdown_t shutdown_flags = 0u;

        if (conn->_vtab->on_read_data == NULL) {
            shutdown_flags |= AH_TCP_SHUTDOWN_RD;
        }
        if (conn->_vtab->on_write_done == NULL) {
            shutdown_flags |= AH_TCP_SHUTDOWN_WR;
        }

        err = ah_tcp_conn_shutdown(conn, shutdown_flags);
    }

    conn->_vtab->on_connect(conn, err);
}

ah_extern ah_err_t ah_tcp_conn_read_start(ah_tcp_conn_t* conn)
{
    if (conn == NULL) {
        return AH_EINVAL;
    }
    if (conn->_state != AH_I_TCP_CONN_STATE_CONNECTED || (conn->_shutdown_flags & AH_TCP_SHUTDOWN_RD) != 0) {
        return AH_ESTATE;
    }

    ah_i_loop_evt_t* evt;
    struct kevent* kev;

    ah_err_t err = ah_i_loop_evt_alloc_with_kev(conn->_loop, &evt, &kev);
    if (err != AH_ENONE) {
        return err;
    }

    evt->_cb = s_on_conn_read;
    evt->_subject = conn;

    EV_SET(kev, conn->_fd, EVFILT_READ, EV_ADD, 0u, 0, evt);
    conn->_read_evt = evt;

    conn->_state = AH_I_TCP_CONN_STATE_READING;

    return AH_ENONE;
}

static void s_on_conn_read(ah_i_loop_evt_t* evt, struct kevent* kev)
{
    ah_assert_if_debug(evt != NULL);
    ah_assert_if_debug(kev != NULL);

    ah_tcp_conn_t* conn = evt->_subject;
    ah_assert_if_debug(conn != NULL);

    if (conn->_state != AH_I_TCP_CONN_STATE_READING) {
        return;
    }

    ah_err_t err;
    ah_bufs_t bufs = { .items = NULL, .length = 0u };

    if (ah_unlikely((kev->flags & EV_ERROR) != 0)) {
        err = (ah_err_t) kev->data;
        goto report_err;
    }

    size_t n_bytes_left = kev->data;

    while (n_bytes_left != 0u) {
        conn->_vtab->on_read_alloc(conn, &bufs);

        if (conn->_state != AH_I_TCP_CONN_STATE_READING) {
            return;
        }

        struct iovec* iov;
        int iovcnt;
        err = ah_i_bufs_into_iovec(&bufs, &iov, &iovcnt);
        if (err != AH_ENONE) {
            goto report_err;
        }

        ssize_t n_bytes_read = readv(conn->_fd, iov, iovcnt);
        if (n_bytes_read < 0) {
            err = errno;
            goto report_err;
        }
        if (n_bytes_read == 0) {
            // We know there are bytes left to read, so the only thing that
            // could cause 0 bytes being read is bufs having no allocated space.
            err = AH_ENOBUFS;
            goto report_err;
        }

        conn->_vtab->on_read_data(conn, bufs, (size_t) n_bytes_read);

        if (conn->_state != AH_I_TCP_CONN_STATE_READING) {
            return;
        }

        n_bytes_left -= (size_t) n_bytes_read;

        // Allocated memory referred to by bufs must be freed by now.
        bufs = (ah_bufs_t) { .items = NULL, .length = 0u };
    }

    if (ah_unlikely((kev->flags & EV_EOF) != 0) && kev->fflags != 0u) {
        err = (ah_err_t) kev->fflags;
        conn->_shutdown_flags |= AH_TCP_SHUTDOWN_RD;
        goto report_err;
    }

    return;

report_err:
    conn->_vtab->on_read_err(conn, err);
}

ah_extern ah_err_t ah_tcp_conn_read_stop(ah_tcp_conn_t* conn)
{
    if (conn == NULL) {
        return AH_EINVAL;
    }
    if (conn->_state != AH_I_TCP_CONN_STATE_READING) {
        return conn->_state == AH_I_TCP_CONN_STATE_CONNECTED ? AH_ESTATE : AH_ENONE;
    }

    struct kevent* kev;
    ah_err_t err = ah_i_loop_alloc_kev(conn->_loop, &kev);
    if (err != AH_ENONE) {
        return err;
    }

    EV_SET(kev, conn->_fd, EVFILT_READ, EV_DELETE, 0, 0u, NULL);

    conn->_state = AH_I_TCP_CONN_STATE_CONNECTED;

    return AH_ENONE;
}

ah_extern ah_err_t ah_tcp_conn_write(ah_tcp_conn_t* conn, ah_tcp_omsg_t* omsg)
{
    if (conn == NULL || omsg == NULL) {
        return AH_EINVAL;
    }
    if (conn->_state < AH_I_TCP_CONN_STATE_CONNECTED || (conn->_shutdown_flags & AH_TCP_SHUTDOWN_WR) != 0) {
        return AH_ESTATE;
    }

    if (conn->_write_queue_head != NULL) {
        conn->_write_queue_end->_next = omsg;
        conn->_write_queue_end = omsg;
        return AH_ENONE;
    }

    conn->_write_queue_head = omsg;
    conn->_write_queue_end = omsg;

    ah_err_t err = s_prep_conn_write(conn);
    if (err != AH_ENONE) {
        return err;
    }

    return AH_ENONE;
}

static ah_err_t s_prep_conn_write(ah_tcp_conn_t* conn)
{
    ah_assert_if_debug(conn != NULL);

    ah_i_loop_evt_t* evt;
    struct kevent* kev;

    ah_err_t err = ah_i_loop_evt_alloc_with_kev(conn->_loop, &evt, &kev);
    if (err != AH_ENONE) {
        return err;
    }

    evt->_cb = s_on_conn_write;
    evt->_subject = conn;

    EV_SET(kev, conn->_fd, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0u, 0, evt);

    return AH_ENONE;
}

static void s_on_conn_write(ah_i_loop_evt_t* evt, struct kevent* kev)
{
    ah_assert_if_debug(evt != NULL);
    ah_assert_if_debug(kev != NULL);

    ah_tcp_conn_t* conn = evt->_subject;
    ah_assert_if_debug(conn != NULL);
    ah_assert_if_debug(conn->_write_queue_head != NULL);

    if (conn->_state < AH_I_TCP_CONN_STATE_CONNECTED) {
        return;
    }

    ah_err_t err;

    ah_tcp_omsg_t* omsg = conn->_write_queue_head;

    if (ah_unlikely((kev->flags & EV_ERROR) != 0)) {
        err = (ah_err_t) kev->data;
        goto report_err_and_prep_next;
    }

    if (ah_unlikely((kev->flags & EV_EOF) != 0)) {
        err = kev->fflags != 0 ? (ah_err_t) kev->fflags : AH_EEOF;
        conn->_shutdown_flags |= AH_TCP_SHUTDOWN_WR;
        goto report_err_and_prep_next;
    }

    ssize_t res = writev(conn->_fd, omsg->_iov, omsg->_iovcnt);
    if (ah_unlikely(res < 0)) {
        err = errno;
        goto report_err_and_prep_next;
    }

    // If more remains to be written but no output buffer space is available,
    // adjust current write buffers and schedule another writing.
    for (int i = 0; i < omsg->_iovcnt; i += 1) {
        struct iovec* iov = &omsg->_iov[i];

        if (((size_t) res) >= iov->iov_len) {
            res -= (ssize_t) iov->iov_len;
            continue;
        }

        // There is more, adjust current write buffers and reschedule.

        omsg->_iov = &omsg->_iov[i];
        omsg->_iovcnt -= i;

        iov->iov_base = &((uint8_t*) iov->iov_base)[(size_t) res];
        iov->iov_len -= (size_t) res;

        goto prep_next;
    }

    // We're done! Dequeue the current write and schedule the next one, if any.
    conn->_write_queue_head = omsg->_next;
    err = AH_ENONE;

report_err_and_prep_next:
    conn->_vtab->on_write_done(conn, err);

    if (conn->_write_queue_head == NULL) {
        return;
    }

prep_next:
    err = s_prep_conn_write(conn);
    if (err != AH_ENONE) {
        conn->_write_queue_head = conn->_write_queue_head->_next;
        goto report_err_and_prep_next;
    }
}

ah_extern ah_err_t ah_tcp_conn_close(ah_tcp_conn_t* conn)
{
    if (conn == NULL) {
        return AH_EINVAL;
    }
    if (conn->_state == AH_I_TCP_CONN_STATE_CLOSED) {
        return AH_ESTATE;
    }
#ifndef NDEBUG
    if (conn->_fd == 0) {
        return AH_ESTATE;
    }
#endif
    conn->_state = AH_I_TCP_CONN_STATE_CLOSED;

    ah_err_t err = ah_i_sock_close(conn->_fd);
    if (err == AH_EINTR) {
        if (ah_i_loop_try_set_pending_err(conn->_loop, AH_EINTR)) {
            err = AH_ENONE;
        }
    }

    if (conn->_read_evt != NULL) {
        ah_i_loop_evt_dealloc(conn->_loop, conn->_read_evt);
    }

#ifndef NDEBUG
    conn->_fd = 0;
#endif

    conn->_vtab->on_close(conn, err);

    return AH_ENONE;
}

ah_extern ah_err_t ah_tcp_listener_listen(ah_tcp_listener_t* ln, unsigned backlog, const ah_tcp_conn_vtab_t* conn_vtab)
{
    if (ln == NULL || conn_vtab == NULL) {
        return AH_EINVAL;
    }
    if (conn_vtab->on_close == NULL) {
        return AH_EINVAL;
    }
    if (conn_vtab->on_read_alloc == NULL || conn_vtab->on_read_data == NULL || conn_vtab->on_read_err == NULL) {
        return AH_EINVAL;
    }
    if (conn_vtab->on_write_done == NULL) {
        return AH_EINVAL;
    }
    if (ln->_state != AH_I_TCP_LISTENER_STATE_OPEN) {
        return AH_ESTATE;
    }

    ah_err_t err;

    int backlog_int = (backlog == 0u ? 16 : backlog <= SOMAXCONN ? (int) backlog : SOMAXCONN);
    if (listen(ln->_fd, backlog_int) != 0) {
        err = errno;
        ln->_vtab->on_listen(ln, err);
        return AH_ENONE;
    }

    ah_i_loop_evt_t* evt;
    struct kevent* kev;

    err = ah_i_loop_evt_alloc_with_kev(ln->_loop, &evt, &kev);
    if (err != AH_ENONE) {
        return err;
    }

    evt->_cb = s_on_listener_accept;
    evt->_subject = ln;

    EV_SET(kev, ln->_fd, EVFILT_READ, EV_ADD, 0u, 0, evt);

    ln->_conn_vtab = conn_vtab;
    ln->_listen_evt = evt;
    ln->_state = AH_I_TCP_LISTENER_STATE_LISTENING;
    ln->_vtab->on_listen(ln, AH_ENONE);

    return AH_ENONE;
}

static void s_on_listener_accept(ah_i_loop_evt_t* evt, struct kevent* kev)
{
    ah_assert_if_debug(evt != NULL);
    ah_assert_if_debug(kev != NULL);

    ah_tcp_listener_t* ln = evt->_subject;
    ah_assert_if_debug(ln != NULL);

    if (ah_unlikely((kev->flags & EV_ERROR) != 0)) {
        ln->_vtab->on_listen(ln, (ah_err_t) kev->data);
        ln->_state = AH_I_TCP_LISTENER_STATE_OPEN;
        return;
    }

    for (int64_t i = 0; i < kev->data; i += 1) {
        ah_tcp_conn_t* conn = NULL;
        ln->_vtab->on_conn_alloc(ln, &conn);
        if (conn == NULL) {
            ln->_vtab->on_conn_err(ln, AH_ENOBUFS);
            continue;
        }

        ah_sockaddr_t sockaddr;
        socklen_t socklen = sizeof(ah_sockaddr_t);

        const int fd = accept(ln->_fd, ah_i_sockaddr_into_bsd(&sockaddr), &socklen);
        if (fd == -1) {
            ln->_vtab->on_conn_err(ln, errno);
            continue;
        }

#if AH_I_SOCKADDR_HAS_SIZE
        ah_assert_if_debug(socklen <= UINT8_MAX);
        sockaddr.as_any.size = socklen;
#endif

        *conn = (ah_tcp_conn_t) {
            ._loop = ln->_loop,
            ._vtab = ln->_conn_vtab,
            ._state = AH_I_TCP_CONN_STATE_CONNECTED,
            ._fd = fd,
        };

        ln->_vtab->on_conn_accept(ln, conn, &sockaddr);
    }

    if (ah_unlikely((kev->flags & EV_EOF) != 0)) {
        ln->_vtab->on_listen(ln, (ah_err_t) kev->fflags != 0 ? (ah_err_t) kev->fflags : AH_EEOF);
        ln->_state = AH_I_TCP_LISTENER_STATE_OPEN;
    }
}

ah_extern ah_err_t ah_tcp_listener_close(ah_tcp_listener_t* ln)
{
    if (ln == NULL) {
        return AH_EINVAL;
    }
    if (ln->_state == AH_I_TCP_LISTENER_STATE_CLOSED) {
        return AH_ESTATE;
    }
#ifndef NDEBUG
    if (ln->_fd == 0) {
        return AH_ESTATE;
    }
#endif
    ln->_state = AH_I_TCP_LISTENER_STATE_CLOSED;

    ah_err_t err = ah_i_sock_close(ln->_fd);
    if (err == AH_EINTR) {
        if (ah_i_loop_try_set_pending_err(ln->_loop, AH_EINTR)) {
            err = AH_ENONE;
        }
    }

    if (ln->_listen_evt != NULL) {
        ah_i_loop_evt_dealloc(ln->_loop, ln->_listen_evt);
    }

#ifndef NDEBUG
    ln->_fd = 0;
#endif

    ln->_vtab->on_close(ln, err);

    return AH_ENONE;
}
