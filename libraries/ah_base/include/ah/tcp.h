// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0.
//
// SPDX-License-Identifier: EPL-2.0

#ifndef AH_TCP_H_
#define AH_TCP_H_

#include "assert.h"
#include "buf.h"
#include "internal/tcp.h"
#include "sock.h"

#include <stdbool.h>

#define AH_TCP_SHUTDOWN_RD   1u
#define AH_TCP_SHUTDOWN_WR   2u
#define AH_TCP_SHUTDOWN_RDWR 3u

typedef unsigned ah_tcp_shutdown_t;

struct ah_tcp_conn {
    AH_I_TCP_CONN_FIELDS
};

struct ah_tcp_conn_vtab {
    void (*on_open)(ah_tcp_conn_t* conn, ah_err_t err);
    void (*on_connect)(ah_tcp_conn_t* conn, ah_err_t err);
    void (*on_close)(ah_tcp_conn_t* conn, ah_err_t err);

    void (*on_read_alloc)(ah_tcp_conn_t* conn, ah_bufs_t* bufs, size_t n_bytes_expected);
    void (*on_read_done)(ah_tcp_conn_t* conn, ah_bufs_t bufs, size_t n_bytes_read, ah_err_t err);

    void (*on_write_done)(ah_tcp_conn_t* conn, ah_bufs_t bufs, size_t n_bytes_written, ah_err_t err);
};

struct ah_tcp_listener {
    AH_I_TCP_LISTENER_FIELDS
};

struct ah_tcp_listener_vtab {
    void (*on_open)(ah_tcp_listener_t* ln, ah_err_t err);
    void (*on_listen)(ah_tcp_listener_t* ln, ah_err_t err);
    void (*on_close)(ah_tcp_listener_t* ln, ah_err_t err);

    void (*on_conn_alloc)(ah_tcp_listener_t* ln, ah_tcp_conn_t** conn, const ah_sockaddr_t* conn_addr);
    void (*on_conn_accept)(ah_tcp_listener_t* ln, ah_tcp_conn_t* conn, const ah_sockaddr_t* conn_addr, ah_err_t err);
};

struct ah_tcp_trans {
    AH_I_TCP_TRANS_FIELDS
};

struct ah_tcp_trans_vtab {
    ah_err_t (*conn_init)(ah_tcp_conn_t* conn, ah_loop_t* loop, const ah_tcp_conn_vtab_t* vtab);
    ah_err_t (*conn_open)(ah_tcp_conn_t* conn, const ah_sockaddr_t* local_addr);
    ah_err_t (*conn_connect)(ah_tcp_conn_t* conn, const ah_sockaddr_t* remote_addr);
    ah_err_t (*conn_read_start)(ah_tcp_conn_t* conn);
    ah_err_t (*conn_read_stop)(ah_tcp_conn_t* conn);
    ah_err_t (*conn_write)(ah_tcp_conn_t* conn, ah_bufs_t bufs); // May modify bufs.items.
    ah_err_t (*conn_shutdown)(ah_tcp_conn_t* conn, ah_tcp_shutdown_t flags);
    ah_err_t (*conn_close)(ah_tcp_conn_t* conn);

    ah_err_t (*listener_init)(ah_tcp_listener_t* ln, ah_loop_t* loop, const ah_tcp_listener_vtab_t* vtab);
    ah_err_t (*listener_open)(ah_tcp_listener_t* ln, const ah_sockaddr_t* local_addr);
    ah_err_t (*listener_listen)(ah_tcp_listener_t* ln, unsigned backlog, const ah_tcp_conn_vtab_t* conn_vtab);
    ah_err_t (*listener_close)(ah_tcp_listener_t* ln);
};

ah_extern ah_err_t ah_tcp_conn_init(ah_tcp_conn_t* conn, ah_loop_t* loop, const ah_tcp_conn_vtab_t* vtab);
ah_extern ah_err_t ah_tcp_conn_open(ah_tcp_conn_t* conn, const ah_sockaddr_t* local_addr);
ah_extern ah_err_t ah_tcp_conn_connect(ah_tcp_conn_t* conn, const ah_sockaddr_t* remote_addr);
ah_extern ah_err_t ah_tcp_conn_read_start(ah_tcp_conn_t* conn);
ah_extern ah_err_t ah_tcp_conn_read_stop(ah_tcp_conn_t* conn);
ah_extern ah_err_t ah_tcp_conn_write(ah_tcp_conn_t* conn, ah_bufs_t bufs); // May modify bufs.items.
ah_extern ah_err_t ah_tcp_conn_shutdown(ah_tcp_conn_t* conn, ah_tcp_shutdown_t flags);
ah_extern ah_err_t ah_tcp_conn_close(ah_tcp_conn_t* conn);

ah_extern ah_err_t ah_tcp_conn_get_local_addr(const ah_tcp_conn_t* conn, ah_sockaddr_t* local_addr);
ah_extern ah_err_t ah_tcp_conn_get_remote_addr(const ah_tcp_conn_t* conn, ah_sockaddr_t* remote_addr);

ah_inline ah_loop_t* ah_tcp_conn_get_loop(const ah_tcp_conn_t* conn)
{
    ah_assert_if_debug(conn != NULL);
    return conn->_loop;
}

ah_inline void* ah_tcp_conn_get_user_data(const ah_tcp_conn_t* conn)
{
    ah_assert_if_debug(conn != NULL);
    return conn->_user_data;
}

ah_extern ah_err_t ah_tcp_conn_set_keepalive(ah_tcp_conn_t* conn, bool is_enabled);
ah_extern ah_err_t ah_tcp_conn_set_nodelay(ah_tcp_conn_t* conn, bool is_enabled);
ah_extern ah_err_t ah_tcp_conn_set_reuseaddr(ah_tcp_conn_t* conn, bool is_enabled);

ah_inline void ah_tcp_conn_set_user_data(ah_tcp_conn_t* conn, void* user_data)
{
    ah_assert_if_debug(conn != NULL);
    conn->_user_data = user_data;
}

ah_extern ah_err_t ah_tcp_listener_init(ah_tcp_listener_t* ln, ah_loop_t* loop, const ah_tcp_listener_vtab_t* vtab);
ah_extern ah_err_t ah_tcp_listener_open(ah_tcp_listener_t* ln, const ah_sockaddr_t* local_addr);
ah_extern ah_err_t ah_tcp_listener_listen(ah_tcp_listener_t* ln, unsigned backlog, const ah_tcp_conn_vtab_t* conn_vtab);
ah_extern ah_err_t ah_tcp_listener_close(ah_tcp_listener_t* ln);

ah_extern ah_err_t ah_tcp_listener_get_local_addr(const ah_tcp_listener_t* ln, ah_sockaddr_t* local_addr);

ah_inline ah_loop_t* ah_tcp_listener_get_loop(const ah_tcp_listener_t* ln)
{
    ah_assert_if_debug(ln != NULL);
    return ln->_loop;
}

ah_inline void* ah_tcp_listener_get_user_data(const ah_tcp_listener_t* ln)
{
    ah_assert_if_debug(ln != NULL);
    return ln->_user_data;
}

ah_extern ah_err_t ah_tcp_listener_set_keepalive(ah_tcp_listener_t* ln, bool is_enabled);
ah_extern ah_err_t ah_tcp_listener_set_nodelay(ah_tcp_listener_t* ln, bool is_enabled);
ah_extern ah_err_t ah_tcp_listener_set_reuseaddr(ah_tcp_listener_t* ln, bool is_enabled);

ah_inline void ah_tcp_listener_set_user_data(ah_tcp_listener_t* ln, void* user_data)
{
    ah_assert_if_debug(ln != NULL);
    ln->_user_data = user_data;
}

ah_extern void ah_tcp_trans_init(ah_tcp_trans_t* trans, ah_loop_t* loop);

#endif
