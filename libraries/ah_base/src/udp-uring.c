// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0.
//
// SPDX-License-Identifier: EPL-2.0

#include "ah/udp.h"

#include "ah/assert.h"
#include "ah/err.h"
#include "ah/loop.h"

static void s_on_sock_close(ah_i_loop_evt_t* evt, struct io_uring_cqe* cqe);
static void s_on_sock_recv(ah_i_loop_evt_t* evt, struct io_uring_cqe* cqe);
static void s_on_sock_send(ah_i_loop_evt_t* evt, struct io_uring_cqe* cqe);

static ah_err_t s_prep_sock_recv(ah_udp_sock_t* sock);

ah_err_t ah_i_udp_sock_recv_start(void* ctx, ah_udp_sock_t* sock)
{
    (void) ctx;

    if (sock == NULL) {
        return AH_EINVAL;
    }
    if (sock->_state != AH_I_UDP_SOCK_STATE_OPEN || sock->_cbs->on_recv_data == NULL) {
        return AH_ESTATE;
    }

    sock->_state = AH_I_UDP_SOCK_STATE_RECEIVING;

    ah_err_t err = s_prep_sock_recv(sock);
    if (err != AH_ENONE) {
        return err;
    }

    return AH_ENONE;
}

static ah_err_t s_prep_sock_recv(ah_udp_sock_t* sock)
{
    ah_assert_if_debug(sock != NULL);

    ah_i_loop_evt_t* evt;
    struct io_uring_sqe* sqe;

    ah_err_t err = ah_i_loop_evt_alloc_with_sqe(sock->_loop, &evt, &sqe);
    if (err != AH_ENONE) {
        return err;
    }

    evt->_cb = s_on_sock_recv;
    evt->_subject = sock;

    sock->_recv_buf = (ah_buf_t) { 0u };
    sock->_cbs->on_recv_alloc(sock, &sock->_recv_buf);

    if (sock->_state != AH_I_UDP_SOCK_STATE_RECEIVING) {
        return AH_ENONE;
    }

    if (ah_buf_is_empty(&sock->_recv_buf)) {
        sock->_state = AH_I_UDP_SOCK_STATE_OPEN;
        return AH_ENOBUFS;
    }

    sock->_recv_msghdr = (struct msghdr) {
        .msg_name = ah_i_sockaddr_into_bsd(&sock->_recv_addr),
        .msg_namelen = sizeof(ah_sockaddr_t),
        .msg_iov = ah_i_buf_into_iovec(&sock->_recv_buf),
        .msg_iovlen = 1,
    };

    io_uring_prep_recvmsg(sqe, sock->_fd, &sock->_recv_msghdr, 0);
    io_uring_sqe_set_data(sqe, evt);

    return AH_ENONE;
}

static void s_on_sock_recv(ah_i_loop_evt_t* evt, struct io_uring_cqe* cqe)
{
    ah_assert_if_debug(evt != NULL);
    ah_assert_if_debug(cqe != NULL);

    ah_udp_sock_t* sock = evt->_subject;
    ah_assert_if_debug(sock != NULL);

    if (sock->_state != AH_I_UDP_SOCK_STATE_RECEIVING) {
        return;
    }

    ah_err_t err;
    ah_sockaddr_t* raddr;

    if (ah_unlikely(cqe->res < 0)) {
        err = -(cqe->res);
        raddr = NULL;
        goto report_err;
    }
    else {
        raddr = ah_i_sockaddr_from_bsd(sock->_recv_msghdr.msg_name);
    }

    sock->_cbs->on_recv_data(sock, sock->_recv_buf, cqe->res, raddr, AH_ENONE);
#ifndef NDEBUG
    sock->_recv_buf = (ah_buf_t) { 0u };
#endif

    if (sock->_state != AH_I_UDP_SOCK_STATE_RECEIVING) {
        return;
    }

    err = s_prep_sock_recv(sock);
    if (err != AH_ENONE) {
        goto report_err;
    }

    return;

report_err:
    sock->_cbs->on_recv_data(sock, (ah_buf_t) { 0u }, 0u, raddr, err);
}

ah_err_t ah_i_udp_sock_recv_stop(void* ctx, ah_udp_sock_t* sock)
{
    (void) ctx;

    if (sock == NULL) {
        return AH_EINVAL;
    }
    if (sock->_state != AH_I_UDP_SOCK_STATE_RECEIVING) {
        return AH_ESTATE;
    }
    sock->_state = AH_I_UDP_SOCK_STATE_OPEN;

    return AH_ENONE;
}

ah_err_t ah_i_udp_sock_send(void* ctx, ah_udp_sock_t* sock, ah_udp_msg_t* msg)
{
    (void) ctx;

    if (sock == NULL || msg == NULL) {
        return AH_EINVAL;
    }
    if (sock->_state < AH_I_UDP_SOCK_STATE_OPEN || sock->_cbs->on_send_done == NULL) {
        return AH_ESTATE;
    }

    ah_i_loop_evt_t* evt;
    struct io_uring_sqe* sqe;

    ah_err_t err = ah_i_loop_evt_alloc_with_sqe(sock->_loop, &evt, &sqe);
    if (err != AH_ENONE) {
        return err;
    }

    evt->_cb = s_on_sock_send;
    evt->_subject = sock;
    evt->_object = (void*) msg->raddr;

    struct msghdr* msghdr = (struct msghdr*) &msg->_msghdr;

    msghdr->msg_name = (void*) ah_i_sockaddr_const_into_bsd(msg->raddr);
    msghdr->msg_namelen = ah_i_sockaddr_get_size(msg->raddr);
    msghdr->msg_iov = ah_i_buf_into_iovec((ah_buf_t*) &msg->buf);
    msghdr->msg_iovlen = 1u;

    io_uring_prep_sendmsg(sqe, sock->_fd, msghdr, 0u);
    io_uring_sqe_set_data(sqe, evt);

    return AH_ENONE;
}

static void s_on_sock_send(ah_i_loop_evt_t* evt, struct io_uring_cqe* cqe)
{
    ah_assert_if_debug(evt != NULL);
    ah_assert_if_debug(cqe != NULL);

    ah_udp_sock_t* sock = evt->_subject;
    ah_assert_if_debug(sock != NULL);

    ah_err_t err;
    size_t n_bytes_sent;

    if (ah_unlikely(cqe->res < 0)) {
        err = -(cqe->res);
        n_bytes_sent = 0u;
    }
    else {
        err = AH_ENONE;
        n_bytes_sent = cqe->res;
    }

    sock->_cbs->on_send_done(sock, n_bytes_sent, evt->_object, err);
}

ah_err_t ah_i_udp_sock_close(void* ctx, ah_udp_sock_t* sock)
{
    (void) ctx;

    if (sock == NULL) {
        return AH_EINVAL;
    }
    if (sock->_state == AH_I_UDP_SOCK_STATE_CLOSED) {
        return AH_ESTATE;
    }
#ifndef NDEBUG
    if (sock->_fd == 0) {
        return AH_ESTATE;
    }
#endif
    sock->_state = AH_I_UDP_SOCK_STATE_CLOSED;

    ah_err_t err;

    ah_i_loop_evt_t* evt;
    struct io_uring_sqe* sqe;

    err = ah_i_loop_evt_alloc_with_sqe(sock->_loop, &evt, &sqe);
    if (err == AH_ENONE) {
        evt->_cb = s_on_sock_close;
        evt->_subject = sock;

        io_uring_prep_close(sqe, sock->_fd);
        io_uring_sqe_set_data(sqe, evt);

        return AH_ENONE;
    }

    // These events are safe to ignore. No other errors should be possible.
    ah_assert_if_debug(err == AH_ENOMEM || err == AH_ENOBUFS || err == AH_ESTATE);

    err = ah_i_sock_close(sock->_fd);
    if (err == AH_EINTR) {
        if (ah_i_loop_try_set_pending_err(sock->_loop, AH_EINTR)) {
            err = AH_ENONE;
        }
    }

#ifndef NDEBUG
    sock->_fd = 0;
#endif

    sock->_cbs->on_close(sock, err);

    return AH_ENONE;
}

static void s_on_sock_close(ah_i_loop_evt_t* evt, struct io_uring_cqe* cqe)
{
    ah_assert_if_debug(evt != NULL);
    ah_assert_if_debug(cqe != NULL);

    ah_udp_sock_t* sock = evt->_subject;
    ah_assert_if_debug(sock != NULL);

#ifndef NDEBUG
    sock->_fd = 0;
#endif

    sock->_cbs->on_close(sock, -(cqe->res));
}
