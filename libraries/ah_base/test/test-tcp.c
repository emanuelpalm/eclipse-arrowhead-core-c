// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0.
//
// SPDX-License-Identifier: EPL-2.0

#include "ah/err.h"
#include "ah/ip.h"
#include "ah/loop.h"
#include "ah/sock.h"
#include "ah/tcp.h"
#include "ah/unit.h"

struct s_tcp_conn_user_data {
    ah_buf_t* free_buf;

    const ah_sockaddr_t* ln_addr;
    ah_tcp_listener_t* ln;

    size_t* close_call_counter;

    bool did_call_open_cb;
    bool did_call_connect_cb;
    bool did_call_close_cb;
    bool did_call_read_alloc_cb;
    bool did_call_read_done_cb;
    bool did_call_write_done_cb;

    ah_unit_t* unit;
};

struct s_tcp_listener_user_data {
    ah_sockaddr_t addr;
    ah_tcp_conn_t *conn;

    ah_tcp_conn_t* free_conn;
    struct s_tcp_conn_user_data accept_user_data;
    ah_tcp_msg_t conn_msg;

    bool did_call_open_cb;
    bool did_call_listen_cb;
    bool did_call_close_cb;
    bool did_call_conn_alloc_cb;
    bool did_call_conn_accept_cb;

    ah_unit_t* unit;
};

static void s_should_read_and_write_data(ah_unit_t* unit);

void test_tcp(ah_unit_t* unit)
{
    s_should_read_and_write_data(unit);
}

static void s_on_conn_open(ah_tcp_conn_t* conn, ah_err_t err);
static void s_on_conn_connect(ah_tcp_conn_t* conn, ah_err_t err);
static void s_on_conn_close(ah_tcp_conn_t* conn, ah_err_t err);
static void s_on_conn_read_alloc(ah_tcp_conn_t* conn, ah_buf_t* buf);
static void s_on_conn_read_data(ah_tcp_conn_t* conn, ah_buf_t buf, size_t nread, ah_err_t err);
static void s_on_conn_write_done(ah_tcp_conn_t* conn, ah_err_t err);

static void s_on_listener_open(ah_tcp_listener_t* ln, ah_err_t err);
static void s_on_listener_listen(ah_tcp_listener_t* ln, ah_err_t err);
static void s_on_listener_close(ah_tcp_listener_t* ln, ah_err_t err);
static void s_on_listener_conn_alloc(ah_tcp_listener_t* ln, ah_tcp_conn_t** conn);
static void s_on_listener_conn_accept(ah_tcp_listener_t* ln, ah_tcp_conn_t* conn, const ah_sockaddr_t* raddr, ah_err_t err);

static const ah_tcp_conn_cbs_t s_conn_cbs = {
    .on_open = s_on_conn_open,
    .on_connect = s_on_conn_connect,
    .on_close = s_on_conn_close,
    .on_read_alloc = s_on_conn_read_alloc,
    .on_read_data = s_on_conn_read_data,
    .on_write_done = s_on_conn_write_done,
};

static const ah_tcp_listener_cbs_t s_listener_cbs = {
    .on_open = s_on_listener_open,
    .on_listen = s_on_listener_listen,
    .on_close = s_on_listener_close,
    .on_conn_alloc = s_on_listener_conn_alloc,
    .on_conn_accept = s_on_listener_conn_accept,
};

static void s_on_conn_open(ah_tcp_conn_t* conn, ah_err_t err)
{
    struct s_tcp_conn_user_data* user_data = ah_tcp_conn_get_user_data(conn);

    if (!ah_unit_assert_err_eq(user_data->unit, AH_ENONE, err)) {
        return;
    }

    err = ah_tcp_conn_set_keepalive(conn, false);
    if (!ah_unit_assert_err_eq(user_data->unit, AH_ENONE, err)) {
        return;
    }

    err = ah_tcp_conn_set_nodelay(conn, true);
    if (!ah_unit_assert_err_eq(user_data->unit, AH_ENONE, err)) {
        return;
    }

    err = ah_tcp_conn_set_reuseaddr(conn, false);
    if (!ah_unit_assert_err_eq(user_data->unit, AH_ENONE, err)) {
        return;
    }

    err = ah_tcp_conn_connect(conn, user_data->ln_addr);
    if (!ah_unit_assert_err_eq(user_data->unit, AH_ENONE, err)) {
        return;
    }

    user_data->did_call_open_cb = true;
}

static void s_on_conn_connect(ah_tcp_conn_t* conn, ah_err_t err)
{
    struct s_tcp_conn_user_data* user_data = ah_tcp_conn_get_user_data(conn);

    if (!ah_unit_assert_err_eq(user_data->unit, AH_ENONE, err)) {
        return;
    }

    err = ah_tcp_conn_read_start(conn);
    if (!ah_unit_assert_err_eq(user_data->unit, AH_ENONE, err)) {
        return;
    }

    user_data->did_call_connect_cb = true;
}

static void s_on_conn_close(ah_tcp_conn_t* conn, ah_err_t err)
{
    struct s_tcp_conn_user_data* user_data = ah_tcp_conn_get_user_data(conn);

    ah_unit_t* unit = user_data->unit;

    if (!ah_unit_assert_err_eq(unit, AH_ENONE, err)) {
        return;
    }

    (*user_data->close_call_counter) += 1u;

    if (*user_data->close_call_counter == 2u) {
        ah_loop_t* loop = ah_tcp_conn_get_loop(conn);
        if (!ah_unit_assert(unit, loop != NULL, "loop == NULL")) {
            return;
        }

        err = ah_loop_term(loop);
        if (!ah_unit_assert_err_eq(unit, AH_ENONE, err)) {
            return;
        }
    }

    user_data->did_call_close_cb = true;
}

static void s_on_conn_read_alloc(ah_tcp_conn_t* conn, ah_buf_t* buf)
{
    struct s_tcp_conn_user_data* user_data = ah_tcp_conn_get_user_data(conn);

    if (!ah_unit_assert(user_data->unit, user_data->free_buf != NULL, "connection read buffer cannot be allocated")) {
        return;
    }

    *buf = *user_data->free_buf;
    user_data->free_buf = NULL;

    user_data->did_call_read_alloc_cb = true;
}

static void s_on_conn_read_data(ah_tcp_conn_t* conn, ah_buf_t buf, size_t nread, ah_err_t err)
{
    struct s_tcp_conn_user_data* user_data = ah_tcp_conn_get_user_data(conn);

    ah_unit_t* unit = user_data->unit;

    if (!ah_unit_assert_err_eq(unit, AH_ENONE, err)) {
        return;
    }

    if (!ah_unit_assert(unit, ah_buf_get_base_const(&buf) != NULL, "ah_buf_get_base_const(buf) == NULL")) {
        return;
    }

    if (!ah_unit_assert_unsigned_eq(unit, 18u, nread)) {
        return;
    }

    if (!ah_unit_assert_unsigned_eq(unit, 24u, ah_buf_get_size(&buf))) {
        return;
    }
    if (!ah_unit_assert_cstr_eq(unit, "Hello, Arrowhead!", (char*) ah_buf_get_base_const(&buf))) {
        return;
    }

    ah_err_t err0 = ah_tcp_conn_close(conn);
    if (!ah_unit_assert_err_eq(unit, AH_ENONE, err0)) {
        return;
    }

    user_data->did_call_read_done_cb = true;
}

static void s_on_conn_write_done(ah_tcp_conn_t* conn, ah_err_t err)
{
    struct s_tcp_conn_user_data* user_data = ah_tcp_conn_get_user_data(conn);

    if (!ah_unit_assert_err_eq(user_data->unit, AH_ENONE, err)) {
        return;
    }

    err = ah_tcp_conn_close(conn);
    if (!ah_unit_assert_err_eq(user_data->unit, AH_ENONE, err)) {
        return;
    }

    err = ah_tcp_listener_close(user_data->ln);
    if (!ah_unit_assert_err_eq(user_data->unit, AH_ENONE, err)) {
        return;
    }

    user_data->did_call_write_done_cb = true;
}

static void s_on_listener_open(ah_tcp_listener_t* ln, ah_err_t err)
{
    struct s_tcp_listener_user_data* user_data = ah_tcp_listener_get_user_data(ln);

    if (!ah_unit_assert_err_eq(user_data->unit, AH_ENONE, err)) {
        return;
    }

    err = ah_tcp_listener_set_nodelay(ln, false);
    if (!ah_unit_assert_err_eq(user_data->unit, AH_ENONE, err)) {
        return;
    }

    err = ah_tcp_listener_listen(ln, 1, &s_conn_cbs);
    if (!ah_unit_assert_err_eq(user_data->unit, AH_ENONE, err)) {
        return;
    }

    user_data->did_call_open_cb = true;
}

static void s_on_listener_listen(ah_tcp_listener_t* ln, ah_err_t err)
{
    struct s_tcp_listener_user_data* user_data = ah_tcp_listener_get_user_data(ln);
    ah_unit_t* unit = user_data->unit;

    if (!ah_unit_assert_err_eq(unit, AH_ENONE, err)) {
        return;
    }

    // Save the IP address the listener is bound to.
    err = ah_tcp_listener_get_laddr(ln, &user_data->addr);
    if (!ah_unit_assert_err_eq(unit, AH_ENONE, err)) {
        return;
    }

    // Open connection that will connect to our listener.
    err = ah_tcp_conn_open(user_data->conn, NULL);
    if (!ah_unit_assert_err_eq(unit, AH_ENONE, err)) {
        return;
    }

    user_data->did_call_listen_cb = true;
}

static void s_on_listener_close(ah_tcp_listener_t* ln, ah_err_t err)
{
    struct s_tcp_listener_user_data* user_data = ah_tcp_listener_get_user_data(ln);

    if (!ah_unit_assert_err_eq(user_data->unit, AH_ENONE, err)) {
        return;
    }

    user_data->did_call_close_cb = true;
}

static void s_on_listener_conn_alloc(ah_tcp_listener_t* ln, ah_tcp_conn_t** conn)
{
    struct s_tcp_listener_user_data* user_data = ah_tcp_listener_get_user_data(ln);

    if (!ah_unit_assert(user_data->unit, user_data->free_conn != NULL, "no free connection can be allocated")) {
        return;
    }

    *conn = user_data->free_conn;
    user_data->free_conn = NULL;

    user_data->did_call_conn_alloc_cb = true;
}

static void s_on_listener_conn_accept(ah_tcp_listener_t* ln, ah_tcp_conn_t* conn, const ah_sockaddr_t* raddr, ah_err_t err)
{
    struct s_tcp_listener_user_data* user_data = ah_tcp_listener_get_user_data(ln);

    ah_unit_t* unit = user_data->unit;

    if (err == AH_ECANCELED) {
        return;
    }

    if (!ah_unit_assert_err_eq(unit, AH_ENONE, err)) {
        return;
    }

    ah_unit_assert(unit, raddr != NULL, "ln_addr == NULL");

    ah_tcp_conn_set_user_data(conn, &user_data->accept_user_data);

    err = ah_buf_init(&user_data->conn_msg.buf, (uint8_t*) "Hello, Arrowhead!", 18u);
    if (!ah_unit_assert_err_eq(unit, AH_ENONE, err)) {
        return;
    }

    err = ah_tcp_conn_write(conn, &user_data->conn_msg);
    if (!ah_unit_assert_err_eq(unit, AH_ENONE, err)) {
        return;
    }

    user_data->did_call_conn_accept_cb = true;
}

static void s_should_read_and_write_data(ah_unit_t* unit)
{
    ah_err_t err;

    // Setup user data.

    size_t close_call_counter = 0u;

    uint8_t free_buf_base[24] = { 0u };
    ah_buf_t free_buf;
    err = ah_buf_init(&free_buf, free_buf_base, sizeof(free_buf_base));
    if (!ah_unit_assert_err_eq(unit, AH_ENONE, err)) {
        return;
    }

    struct s_tcp_conn_user_data conn_user_data = {
        .free_buf = &free_buf,
        .close_call_counter = &close_call_counter,
        .unit = unit,
    };

    struct s_tcp_listener_user_data ln_user_data = {
        .free_conn = &(ah_tcp_conn_t) { 0u },
        .accept_user_data = (struct s_tcp_conn_user_data) {
            .close_call_counter = &close_call_counter,
            .unit = unit,
        },
        .unit = unit,
    };

    // Setup event loop.

    ah_loop_t loop;

    err = ah_loop_init(&loop, &(ah_loop_opts_t) { .capacity = 4u });
    if (!ah_unit_assert_err_eq(unit, AH_ENONE, err)) {
        return;
    }

    // Setup listener.

    ah_tcp_listener_t ln;
    err = ah_tcp_listener_init(&ln, &loop, ah_tcp_trans_get_default(), &s_listener_cbs);
    if (!ah_unit_assert_err_eq(unit, AH_ENONE, err)) {
        return;
    }

    ln_user_data.accept_user_data.ln = &ln;
    ah_tcp_listener_set_user_data(&ln, &ln_user_data);

    // Setup connection.

    ah_tcp_conn_t conn;
    err = ah_tcp_conn_init(&conn, &loop, ah_tcp_trans_get_default(), &s_conn_cbs);
    if (!ah_unit_assert_err_eq(unit, AH_ENONE, err)) {
        return;
    }

    conn_user_data.ln_addr = &ln_user_data.addr;
    ah_tcp_conn_set_user_data(&conn, &conn_user_data);

    ln_user_data.conn = &conn;

    // Open listener, which will open the connection, and so on.

    err = ah_tcp_listener_open(&ln, NULL);
    if (!ah_unit_assert_err_eq(unit, AH_ENONE, err)) {
        return;
    }

    // Submit issued events for execution.

    ah_time_t deadline;
    err = ah_time_add(ah_time_now(), 1 * AH_TIMEDIFF_S, &deadline);
    if (!ah_unit_assert_err_eq(unit, AH_ENONE, err)) {
        return;
    }
    err = ah_loop_run_until(&loop, &deadline);
    if (!ah_unit_assert_err_eq(unit, AH_ENONE, err)) {
        return;
    }

    // Check results.

    struct s_tcp_conn_user_data* conn_data = &conn_user_data;
    (void) ah_unit_assert(unit, conn_data->did_call_open_cb, "`conn` s_on_conn_open() not called");
    (void) ah_unit_assert(unit, conn_data->did_call_connect_cb, "`conn` s_on_conn_connect() not called");
    (void) ah_unit_assert(unit, conn_data->did_call_close_cb, "`conn` s_on_conn_close() not called");
    (void) ah_unit_assert(unit, conn_data->did_call_read_alloc_cb, "`conn` s_on_conn_read_alloc() not called");
    (void) ah_unit_assert(unit, conn_data->did_call_read_done_cb, "`conn` s_on_conn_read_data() not called");
    (void) ah_unit_assert(unit, !conn_data->did_call_write_done_cb, "`conn` s_on_conn_write_done() was called");

    struct s_tcp_listener_user_data* ln_data = &ln_user_data;
    (void) ah_unit_assert(unit, ln_data->did_call_open_cb, "`ln` s_on_listener_open() not called");
    (void) ah_unit_assert(unit, ln_data->did_call_listen_cb, "`ln` s_on_listener_listen() not called");
    (void) ah_unit_assert(unit, ln_data->did_call_close_cb, "`ln` s_on_listener_close() not called");
    (void) ah_unit_assert(unit, ln_data->did_call_conn_alloc_cb, "`ln` s_on_listener_conn_alloc() not called");
    (void) ah_unit_assert(unit, ln_data->did_call_conn_accept_cb, "`ln` s_on_listener_conn_accept() not called");

    struct s_tcp_conn_user_data* acc_data = &ln_data->accept_user_data;
    (void) ah_unit_assert(unit, !acc_data->did_call_open_cb, "`acc` s_on_conn_open() was called");
    (void) ah_unit_assert(unit, !acc_data->did_call_connect_cb, "`acc` s_on_conn_connect() was called");
    (void) ah_unit_assert(unit, acc_data->did_call_close_cb, "`acc` s_on_conn_close() not called");
    (void) ah_unit_assert(unit, !acc_data->did_call_read_alloc_cb, "`acc` s_on_conn_read_alloc() was called");
    (void) ah_unit_assert(unit, !acc_data->did_call_read_done_cb, "`acc` s_on_conn_read_data() was called");
    (void) ah_unit_assert(unit, acc_data->did_call_write_done_cb, "`acc` s_on_conn_write_done() not called");

    ah_unit_assert(unit, ah_loop_is_term(&loop), "`loop` never terminated");
}
