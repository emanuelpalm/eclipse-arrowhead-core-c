// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0.
//
// SPDX-License-Identifier: EPL-2.0

#include "ah/http.h"

#include "http-client.h"
#include "http-utils.h"

#include <ah/assert.h>
#include <ah/err.h>
#include <ah/math.h>

static void s_on_listener_open(ah_tcp_listener_t* ln, ah_err_t err);
static void s_on_listener_listen(ah_tcp_listener_t* ln, ah_err_t err);
static void s_on_listener_close(ah_tcp_listener_t* ln, ah_err_t err);
static void s_on_listener_conn_alloc(ah_tcp_listener_t* ln, ah_tcp_conn_t** conn);
static void s_on_listener_conn_accept(ah_tcp_listener_t* ln, ah_tcp_conn_t* conn, const ah_sockaddr_t* raddr, ah_err_t err);

ah_extern ah_err_t ah_http_server_init(ah_http_server_t* srv, ah_loop_t* loop, ah_tcp_trans_t trans, const ah_http_server_cbs_t* cbs)
{
    if (srv == NULL || loop == NULL || trans.vtab == NULL || cbs == NULL) {
        return AH_EINVAL;
    }
    if (trans.vtab->conn_open == NULL || trans.vtab->conn_connect == NULL || trans.vtab->conn_read_start == NULL || trans.vtab->conn_read_stop == NULL) {
        return AH_EINVAL;
    }
    if (trans.vtab->conn_write == NULL || trans.vtab->conn_shutdown == NULL || trans.vtab->conn_close == NULL) {
        return AH_EINVAL;
    }
    if (trans.vtab->listener_open == NULL || trans.vtab->listener_listen == NULL || trans.vtab->listener_close == NULL) {
        return AH_EINVAL;
    }
    if (cbs->on_open == NULL || cbs->on_listen == NULL || cbs->on_close == NULL) {
        return AH_EINVAL;
    }
    if (cbs->on_client_alloc == NULL || cbs->on_client_accept == NULL) {
        return AH_EINVAL;
    }

    static const ah_tcp_listener_cbs_t s_cbs = {
        .on_open = s_on_listener_open,
        .on_listen = s_on_listener_listen,
        .on_close = s_on_listener_close,
        .on_conn_alloc = s_on_listener_conn_alloc,
        .on_conn_accept = s_on_listener_conn_accept,
    };

    *srv = (ah_http_server_t) {
        ._cbs = cbs,
    };

    return ah_tcp_listener_init(&srv->_ln, loop, trans, &s_cbs);
}

ah_extern ah_err_t ah_http_server_open(ah_http_server_t* srv, const ah_sockaddr_t* laddr)
{
    if (srv == NULL) {
        return AH_EINVAL;
    }
    return ah_tcp_listener_open(&srv->_ln, laddr);
}

static void s_on_listener_open(ah_tcp_listener_t* ln, ah_err_t err)
{
    ah_http_server_t* srv = ah_i_http_conn_to_server(ln);
    srv->_cbs->on_open(srv, err);
}

ah_extern ah_err_t ah_http_server_listen(ah_http_server_t* srv, unsigned backlog, const ah_http_client_cbs_t* cbs)
{
    if (srv == NULL || cbs == NULL) {
        return AH_EINVAL;
    }
    if (cbs->on_close == NULL || cbs->on_alloc == NULL) {
        return AH_EINVAL;
    }
    if (cbs->on_send_done == NULL || cbs->on_recv_line == NULL || cbs->on_recv_header == NULL || cbs->on_recv_data == NULL || cbs->on_recv_end == NULL) {
        return AH_EINVAL;
    }

    ah_assert_if_debug(cbs->on_close != NULL);
    ah_assert_if_debug(cbs->on_alloc != NULL);
    ah_assert_if_debug(cbs->on_send_done != NULL);
    ah_assert_if_debug(cbs->on_recv_line != NULL);
    ah_assert_if_debug(cbs->on_recv_header != NULL);
    ah_assert_if_debug(cbs->on_recv_data != NULL);
    ah_assert_if_debug(cbs->on_recv_end != NULL);

    srv->_client_cbs = cbs;

    return ah_tcp_listener_listen(&srv->_ln, backlog, ah_i_http_client_get_conn_cbs());
}

static void s_on_listener_listen(ah_tcp_listener_t* ln, ah_err_t err)
{
    ah_http_server_t* srv = ah_i_http_conn_to_server(ln);
    srv->_cbs->on_listen(srv, err);
}

static void s_on_listener_conn_alloc(ah_tcp_listener_t* ln, ah_tcp_conn_t** conn)
{
    ah_assert_if_debug(conn != NULL);

    ah_http_server_t* srv = ah_i_http_conn_to_server(ln);

    ah_http_client_t* cln;
    srv->_cbs->on_client_alloc(srv, &cln);
    if (cln != NULL) {
        *cln = (ah_http_client_t) { 0u };
        *conn = &cln->_conn;
    }
}

static void s_on_listener_conn_accept(ah_tcp_listener_t* ln, ah_tcp_conn_t* conn, const ah_sockaddr_t* raddr, ah_err_t err)
{
    ah_http_server_t* srv = ah_i_http_conn_to_server(ln);
    if (err != AH_ENONE) {
        srv->_cbs->on_client_accept(srv, NULL, err);
        return;
    }

    ah_http_client_t* cln = ah_i_http_conn_to_client(conn);
    ah_i_http_client_init_accepted(cln, srv, raddr);

    srv->_cbs->on_client_accept(srv, cln, err);
    if (ah_tcp_conn_is_closed(conn)) {
        return;
    }

    err = ah_tcp_conn_read_start(conn);
    if (err != AH_ENONE) {
        cln->_cbs->on_recv_end(cln, err);
    }
}

ah_extern ah_err_t ah_http_server_close(ah_http_server_t* srv)
{
    if (srv == NULL) {
        return AH_EINVAL;
    }
    return ah_tcp_listener_close(&srv->_ln);
}

static void s_on_listener_close(ah_tcp_listener_t* ln, ah_err_t err)
{
    ah_http_server_t* srv = ah_i_http_conn_to_server(ln);
    srv->_cbs->on_close(srv, err);
}

ah_extern ah_tcp_listener_t* ah_http_server_get_listener(ah_http_server_t* srv)
{
    ah_assert(srv != NULL);

    return &srv->_ln;
}

ah_extern ah_err_t ah_http_server_get_laddr(const ah_http_server_t* srv, ah_sockaddr_t* laddr)
{
    ah_assert(srv != NULL);

    return ah_tcp_listener_get_laddr(&srv->_ln, laddr);
}

ah_extern ah_loop_t* ah_http_server_get_loop(const ah_http_server_t* srv)
{
    ah_assert(srv != NULL);

    return ah_tcp_listener_get_loop(&srv->_ln);
}

ah_extern void* ah_http_server_get_user_data(const ah_http_server_t* srv)
{
    ah_assert(srv != NULL);

    return ah_tcp_listener_get_user_data(&srv->_ln);
}

ah_extern void ah_http_server_set_user_data(ah_http_server_t* srv, void* user_data)
{
    ah_assert(srv != NULL);

    ah_tcp_listener_set_user_data(&srv->_ln, user_data);
}
