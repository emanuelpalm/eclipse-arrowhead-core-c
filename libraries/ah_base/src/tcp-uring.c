// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0.
//
// SPDX-License-Identifier: EPL-2.0

#include "ah/tcp.h"

#include "ah/assert.h"
#include "ah/err.h"
#include "ah/loop.h"

#include <stddef.h>
#include <sys/socket.h>

static void s_on_conn_connect(ah_i_loop_evt_t* evt, struct io_uring_cqe* cqe);
static void s_on_conn_read(ah_i_loop_evt_t* evt, struct io_uring_cqe* cqe);
static void s_on_conn_write(ah_i_loop_evt_t* evt, struct io_uring_cqe* cqe);
static void s_on_conn_close(ah_i_loop_evt_t* evt, struct io_uring_cqe* cqe);

static void s_on_listener_accept(ah_i_loop_evt_t* evt, struct io_uring_cqe* cqe);
static void s_on_listener_close(ah_i_loop_evt_t* evt, struct io_uring_cqe* cqe);

static ah_err_t s_prep_conn_read(ah_tcp_conn_t* conn);

ah_err_t ah_i_tcp_conn_connect(void* ctx, ah_tcp_conn_t* conn, const ah_sockaddr_t* raddr)
{
    (void) ctx;

    if (conn == NULL || raddr == NULL || !ah_sockaddr_is_ip(raddr)) {
        return AH_EINVAL;
    }
    if (conn->_state != AH_I_TCP_CONN_STATE_OPEN) {
        return AH_ESTATE;
    }

    ah_i_loop_evt_t* evt;
    struct io_uring_sqe* sqe;

    ah_err_t err = ah_i_loop_evt_alloc_with_sqe(conn->_loop, &evt, &sqe);
    if (err != AH_ENONE) {
        return err;
    }

    evt->_cb = s_on_conn_connect;
    evt->_subject = conn;

    io_uring_prep_connect(sqe, conn->_fd, ah_i_sockaddr_const_into_bsd(raddr), ah_i_sockaddr_get_size(raddr));
    io_uring_sqe_set_data(sqe, evt);

    conn->_state = AH_I_TCP_CONN_STATE_CONNECTING;

    return AH_ENONE;
}

static void s_on_conn_connect(ah_i_loop_evt_t* evt, struct io_uring_cqe* cqe)
{
    ah_assert_if_debug(evt != NULL);
    ah_assert_if_debug(cqe != NULL);

    ah_tcp_conn_t* conn = evt->_subject;
    ah_assert_if_debug(conn != NULL);

    ah_err_t err;

    if (ah_likely(cqe->res == 0)) {
        conn->_state = AH_I_TCP_CONN_STATE_CONNECTED;

        ah_tcp_shutdown_t shutdown_flags = 0u;

        if (conn->_cbs->on_read == NULL) {
            shutdown_flags |= AH_TCP_SHUTDOWN_RD;
        }
        if (conn->_cbs->on_write == NULL) {
            shutdown_flags |= AH_TCP_SHUTDOWN_WR;
        }
        if (shutdown_flags != 0) {
            err = ah_tcp_conn_shutdown(conn, shutdown_flags);
        }
        else {
            err = AH_ENONE;
        }
    }
    else {
        conn->_state = AH_I_TCP_CONN_STATE_OPEN;
        err = -(cqe->res);
    }

    conn->_cbs->on_connect(conn, err);
}

ah_err_t ah_i_tcp_conn_read_start(void* ctx, ah_tcp_conn_t* conn)
{
    (void) ctx;

    if (conn == NULL) {
        return AH_EINVAL;
    }
    if (conn->_state != AH_I_TCP_CONN_STATE_CONNECTED || (conn->_shutdown_flags & AH_TCP_SHUTDOWN_RD) != 0) {
        return AH_ESTATE;
    }

    conn->_state = AH_I_TCP_CONN_STATE_READING;

    ah_err_t err = s_prep_conn_read(conn);
    if (err != AH_ENONE) {
        return err;
    }

    return AH_ENONE;
}

static ah_err_t s_prep_conn_read(ah_tcp_conn_t* conn)
{
    ah_assert_if_debug(conn != NULL);

    ah_i_loop_evt_t* evt;
    struct io_uring_sqe* sqe;

    ah_err_t err = ah_i_loop_evt_alloc_with_sqe(conn->_loop, &evt, &sqe);
    if (err != AH_ENONE) {
        return err;
    }

    evt->_cb = s_on_conn_read;
    evt->_subject = conn;

    conn->_read_evt = evt;
    conn->_read_page = ah_palloc();

    if (conn->_read_page == NULL) {
        conn->_state = AH_I_TCP_CONN_STATE_CONNECTED;
        return AH_ENOMEM;
    }


    io_uring_prep_recv(sqe, conn->_fd, conn->_read_page, AH_PSIZE, 0);
    io_uring_sqe_set_data(sqe, evt);

    return AH_ENONE;
}

static void s_on_conn_read(ah_i_loop_evt_t* evt, struct io_uring_cqe* cqe)
{
    ah_assert_if_debug(evt != NULL);
    ah_assert_if_debug(cqe != NULL);

    ah_tcp_conn_t* conn = evt->_subject;
    ah_assert_if_debug(conn != NULL);

    conn->_read_evt = NULL;

    if (conn->_state != AH_I_TCP_CONN_STATE_READING) {
        return;
    }

    ah_err_t err;

    if (ah_unlikely(cqe->res < 0)) {
        err = -(cqe->res);
        goto report_err;
    }

    if (ah_unlikely(cqe->res == 0)) {
        conn->_shutdown_flags |= AH_TCP_SHUTDOWN_RD;
        err = AH_EEOF;
        goto report_err;
    }

    if (ah_unlikely(AH_PSIZE < (size_t) cqe->res)) {
        err = AH_EDOM;
        goto report_err;
    }

    conn->_cbs->on_read(conn, conn->_read_page, cqe->res, AH_ENONE);

    ah_pfree(conn->_read_page);

#ifndef NDEBUG
    conn->_read_page = NULL;
#endif

    if (conn->_state != AH_I_TCP_CONN_STATE_READING) {
        return;
    }

    err = s_prep_conn_read(conn);
    if (err != AH_ENONE) {
        goto report_err;
    }

    return;

report_err:
    conn->_cbs->on_read(conn, NULL, 0u, err);
}

ah_err_t ah_i_tcp_conn_read_stop(void* ctx, ah_tcp_conn_t* conn)
{
    (void) ctx;

    if (conn == NULL) {
        return AH_EINVAL;
    }
    if (conn->_state != AH_I_TCP_CONN_STATE_READING) {
        return conn->_state == AH_I_TCP_CONN_STATE_CONNECTED ? AH_ESTATE : AH_ENONE;
    }
    conn->_state = AH_I_TCP_CONN_STATE_CONNECTED;

    if (conn->_read_evt != NULL) {
        struct io_uring_sqe* sqe;
        if (ah_i_loop_alloc_sqe(conn->_loop, &sqe) == AH_ENONE) {
            io_uring_prep_cancel(sqe, conn->_read_evt, 0);
            conn->_read_evt = NULL;
        }
    }

    return AH_ENONE;
}

ah_err_t ah_i_tcp_conn_write(void* ctx, ah_tcp_conn_t* conn, ah_tcp_out_t* out)
{
    (void) ctx;

    if (conn == NULL || out == NULL) {
        return AH_EINVAL;
    }
    if (conn->_state < AH_I_TCP_CONN_STATE_CONNECTED || (conn->_shutdown_flags & AH_TCP_SHUTDOWN_WR) != 0) {
        return AH_ESTATE;
    }

    ah_i_loop_evt_t* evt;
    struct io_uring_sqe* sqe;

    ah_err_t err = ah_i_loop_evt_alloc_with_sqe(conn->_loop, &evt, &sqe);
    if (err != AH_ENONE) {
        return err;
    }

    evt->_cb = s_on_conn_write;
    evt->_subject = conn;

    io_uring_prep_send(sqe, conn->_fd, out->buf._base, out->buf._size, 0);
    io_uring_sqe_set_data(sqe, evt);

    return AH_ENONE;
}

static void s_on_conn_write(ah_i_loop_evt_t* evt, struct io_uring_cqe* cqe)
{
    ah_assert_if_debug(evt != NULL);
    ah_assert_if_debug(cqe != NULL);

    ah_tcp_conn_t* conn = evt->_subject;
    ah_assert_if_debug(conn != NULL);

    if (conn->_state < AH_I_TCP_CONN_STATE_CONNECTED) {
        return;
    }

    ah_err_t err;

    if (ah_unlikely(cqe->res < 0)) {
        err = -(cqe->res);
    }
    else {
        err = AH_ENONE;
    }

    conn->_cbs->on_write(conn, err);
}

ah_err_t ah_i_tcp_conn_close(void* ctx, ah_tcp_conn_t* conn)
{
    (void) ctx;

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

    ah_err_t err;

    ah_i_loop_evt_t* evt;
    struct io_uring_sqe* sqe;

    err = ah_i_loop_evt_alloc_with_sqe(conn->_loop, &evt, &sqe);

    if (err == AH_ENONE) {
        evt->_cb = s_on_conn_close;
        evt->_subject = conn;

        io_uring_prep_close(sqe, conn->_fd);
        io_uring_sqe_set_data(sqe, evt);

        return AH_ENONE;
    }

    // These events are safe to ignore. No other errors should be possible.
    ah_assert_if_debug(err == AH_ENOMEM || err == AH_ENOBUFS || err == AH_ESTATE);

    err = ah_i_sock_close(conn->_fd);
    if (err == AH_EINTR) {
        if (ah_i_loop_try_set_pending_err(conn->_loop, AH_EINTR)) {
            err = AH_ENONE;
        }
    }

#ifndef NDEBUG
    conn->_fd = 0;
#endif

    conn->_shutdown_flags = AH_TCP_SHUTDOWN_RDWR;
    conn->_cbs->on_close(conn, err);

    return AH_ENONE;
}

static void s_on_conn_close(ah_i_loop_evt_t* evt, struct io_uring_cqe* cqe)
{
    ah_assert_if_debug(evt != NULL);
    ah_assert_if_debug(cqe != NULL);

    ah_tcp_conn_t* conn = evt->_subject;
    ah_assert_if_debug(conn != NULL);

    conn->_shutdown_flags = AH_TCP_SHUTDOWN_RDWR;
#ifndef NDEBUG
    conn->_fd = 0;
#endif

    conn->_cbs->on_close(conn, -(cqe->res));

    if (conn->_owning_slab != NULL) {
        ah_i_slab_free(conn->_owning_slab, conn);
    }
}

ah_err_t ah_i_tcp_listener_listen(void* ctx, ah_tcp_listener_t* ln, unsigned backlog, const ah_tcp_conn_cbs_t* conn_cbs)
{
    (void) ctx;

    if (ln == NULL || conn_cbs == NULL) {
        return AH_EINVAL;
    }
    if (conn_cbs->on_close == NULL || conn_cbs->on_read == NULL || conn_cbs->on_write == NULL) {
        return AH_EINVAL;
    }
    if (ln->_state != AH_I_TCP_LISTENER_STATE_OPEN) {
        return AH_ESTATE;
    }

    ah_err_t err;

    int backlog_int = (backlog == 0u ? 16 : backlog <= SOMAXCONN ? (int) backlog
                                                                 : SOMAXCONN);
    if (listen(ln->_fd, backlog_int) != 0) {
        err = errno;
        ln->_cbs->on_listen(ln, err);
        return AH_ENONE;
    }

    ah_i_loop_evt_t* evt;
    struct io_uring_sqe* sqe;

    err = ah_i_loop_evt_alloc_with_sqe(ln->_loop, &evt, &sqe);
    if (err != AH_ENONE) {
        return err;
    }

    evt->_cb = s_on_listener_accept;
    evt->_subject = ln;

    ln->_raddr_len = sizeof(ah_sockaddr_t);
    io_uring_prep_accept(sqe, ln->_fd, ah_i_sockaddr_into_bsd(&ln->_raddr), &ln->_raddr_len, 0);
    io_uring_sqe_set_data(sqe, evt);

    ln->_conn_cbs = conn_cbs;
    ln->_state = AH_I_TCP_LISTENER_STATE_LISTENING;
    ln->_cbs->on_listen(ln, AH_ENONE);

    return AH_ENONE;
}

static void s_on_listener_accept(ah_i_loop_evt_t* evt, struct io_uring_cqe* cqe)
{
    ah_assert_if_debug(evt != NULL);
    ah_assert_if_debug(cqe != NULL);

    ah_tcp_listener_t* ln = evt->_subject;
    ah_assert_if_debug(ln != NULL);

    if (ah_unlikely(cqe->res < 0)) {
        ln->_cbs->on_accept(ln, NULL, NULL, -cqe->res);
        goto prep_another_accept;
    }

    ah_tcp_conn_t* conn = ah_i_slab_alloc(&ln->_conn_slab);
    if (conn == NULL) {
        ln->_cbs->on_accept(ln, NULL, NULL, AH_ENOMEM);
        goto prep_another_accept;
    }

    *conn = (ah_tcp_conn_t) {
        ._loop = ln->_loop,
        ._trans = ln->_trans,
        ._owning_slab = &ln->_conn_slab,
        ._cbs = ln->_conn_cbs,
        ._state = AH_I_TCP_CONN_STATE_CONNECTED,
        ._fd = cqe->res,
    };

    ln->_cbs->on_accept(ln, conn, &ln->_raddr, AH_ENONE);

    ah_err_t err;
    ah_i_loop_evt_t* evt0;
    struct io_uring_sqe* sqe;

prep_another_accept:
    if (ah_tcp_listener_is_closed(ln)) {
        return;
    }

    err = ah_i_loop_evt_alloc_with_sqe(ln->_loop, &evt0, &sqe);
    if (err != AH_ENONE) {
        if (err != AH_ECANCELED) {
            ln->_cbs->on_listen(ln, err);
        }
        return;
    }

    evt0->_cb = s_on_listener_accept;
    evt0->_subject = ln;

    ln->_raddr_len = sizeof(ah_sockaddr_t);
    io_uring_prep_accept(sqe, ln->_fd, ah_i_sockaddr_into_bsd(&ln->_raddr), &ln->_raddr_len, 0);
    io_uring_sqe_set_data(sqe, evt0);
}

ah_err_t ah_i_tcp_listener_close(void* ctx, ah_tcp_listener_t* ln)
{
    (void) ctx;

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

    ah_err_t err;

    ah_i_loop_evt_t* evt;
    struct io_uring_sqe* sqe;

    err = ah_i_loop_evt_alloc_with_sqe(ln->_loop, &evt, &sqe);

    if (err == AH_ENONE) {
        evt->_cb = s_on_listener_close;
        evt->_subject = ln;

        io_uring_prep_close(sqe, ln->_fd);
        io_uring_sqe_set_data(sqe, evt);

        return AH_ENONE;
    }

    // These events are safe to ignore. No other errors should be possible.
    ah_assert_if_debug(err == AH_ENOMEM || err == AH_ENOBUFS || err == AH_ECANCELED);

    err = ah_i_sock_close(ln->_fd);
    if (err == AH_EINTR) {
        if (ah_i_loop_try_set_pending_err(ln->_loop, AH_EINTR)) {
            err = AH_ENONE;
        }
    }

#ifndef NDEBUG
    ln->_fd = 0;
#endif

    ln->_cbs->on_close(ln, err);
    ah_i_slab_term(&ln->_conn_slab, NULL);

    return AH_ENONE;
}

static void s_on_listener_close(ah_i_loop_evt_t* evt, struct io_uring_cqe* cqe)
{
    ah_assert_if_debug(evt != NULL);
    ah_assert_if_debug(cqe != NULL);

    ah_tcp_listener_t* ln = evt->_subject;
    ah_assert_if_debug(ln != NULL);

#ifndef NDEBUG
    ln->_fd = 0;
#endif

    ln->_cbs->on_close(ln, -(cqe->res));
    ah_i_slab_term(&ln->_conn_slab, NULL);
}
