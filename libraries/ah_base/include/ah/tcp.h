// SPDX-License-Identifier: EPL-2.0

#ifndef AH_TCP_H_
#define AH_TCP_H_

/**
 * @file
 * Transmission Control Protocol (TCP) utilities.
 *
 * Here, the data structures and functions required to setup and send messages
 * through TCP connections are made available. Such connections are produced
 * either by @e connecting to a remote host or @e listening for incoming
 * connections. To give you more control over the medium through which the
 * connections are made, we also provide a mechanism we refer to as
 * @e transports. Please refer to <a href="https://www.rfc-editor.org/rfc/rfc9293.html">RFC9293</a>
 * to learn more about TCP itself. Below, we briefly describe how to use this C
 * API.
 *
 * @note When we use the terms @e remote and @e local throughout this file, we
 *       do so from the perspective of individual connections rather than
 *       complete devices. In other words, when we consider a certain
 *       connection, that connection is local and whatever listener it connects
 *       to is remote. When we, on the other hand, consider a certain
 *       listener, that listener is local and whatever connection attempts it
 *       receives are remote. Whether the connections and listeners are
 *       physically located on different devices or processes is not of
 *       concern.
 *
 * <h3>Connections</h3>
 *
 * To set up a local connection to a remote listener, you call
 * ah_tcp_conn_init(), ah_tcp_conn_open() and ah_tcp_conn_connect(), in that
 * order. Successfully initialized connections are terminated with
 * ah_tcp_conn_term() and successfully opened connections are closed with
 * ah_tcp_conn_close(). If you wish to configure a connection by setting any of
 * its options, it is typically most appropriate to do so after it has been
 * opened and before it is connected. Every connection may receive data, and is
 * notified of other events, via a <em>connection observer</em> of type
 * ah_tcp_conn_obs.
 *
 * After being successfully connected to a remote host, a connection does not
 * automatically enable receiving of data from its peer. The receiving of peer
 * data must be enabled and disabled using ah_tcp_conn_read_start() and
 * ah_tcp_conn_read_stop() functions. You may also chose to shut down reading
 * and/or writing using ah_tcp_conn_shutdown() when you know nothing else of
 * interest will be read and/or written.
 *
 * To write data to the remote peer of a connection, use the ah_tcp_conn_write()
 * function.
 *
 * <h3>Listeners</h3>
 *
 * Setting up a local TCP listener requires you to call ah_tcp_listener_init(),
 * ah_tcp_listener_open() and ah_tcp_listener_listen(), in that order.
 * Successfully initialized listeners are terminated with ah_tcp_listener_term()
 * and successfully opened listeners are closed with ah_tcp_listener_close(). If
 * you wish to configure a listener by setting any of its options, such as by
 * using ah_tcp_listener_set_reuseaddr(), it is typically most appropriate to do
 * so after the listener has been opened and before it starts to listen for
 * incoming connections. Every listener receives incoming connections and is
 * notified of other events via its <em>listener observer</em>, which is of type
 * ah_tcp_listener_obs.
 *
 * <h3>Transports</h3>
 *
 * Data read and written using this API is concretely handled by a @e transport,
 * represented by the ah_tcp_trans type, which is chosen whenever a given
 * connection or listener is first initialized. The transport receives most
 * function calls related to that connection or receiver, and is free to
 * implement these functions however it sees fit.
 *
 * Most uses of this API will involve the <em>default transport</em>, which will
 * engage the actual TCP networking capabilities of the underlying platform.
 * However, other transports may be used. For example, the ah_mbedtls library
 * provides a TLS/SSL transport, which can be used to establish encrypted
 * connections. Transports may chose to utilize other transports. In fact,
 * the ah_mbedtls transport we just mentioned must be given another transport to
 * function. You may want to use custom transports for testing purposes,
 * logging, compression and decompression, and so on.
 *
 * @see https://www.rfc-editor.org/rfc/rfc9293.html
 */

#include "buf.h"
#include "internal/_tcp.h"
#include "rw.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * The maximum payload size of an ah_tcp_in instance allocated via
 * ah_tcp_in_alloc_for().
 */
#define AH_TCP_IN_BUF_SIZE (AH_PSIZE - sizeof(ah_tcp_in_t))

/**
 * The maximum payload size of an ah_tcp_out instance allocated via
 * ah_tcp_out_alloc().
 */
#define AH_TCP_OUT_BUF_SIZE (AH_PSIZE - sizeof(ah_tcp_out_t))

/**
 * Read shutdown flag that can be provided as argument to
 * ah_tcp_conn_shutdown().
 */
#define AH_TCP_SHUTDOWN_RD 1u

/**
 * Write shutdown flag that can be provided as argument to
 * ah_tcp_conn_shutdown().
 */
#define AH_TCP_SHUTDOWN_WR 2u

/**
 * Read and write shutdown flag that can be provided as argument to
 * ah_tcp_conn_shutdown().
 */
#define AH_TCP_SHUTDOWN_RDWR 3u

/**
 * TCP-based transport.
 *
 * A @e transport represents a medium through which TCP connections can be
 * established. Such a medium could be a plain connection via an underlying
 * operating system, a TLS/SSL layer on top of a plain connection, etc.
 * Transports may pass on their
 */
struct ah_tcp_trans {
    /** Virtual function table used to interact with transport medium. */
    const ah_tcp_trans_vtab_t* vtab;

    /** Pointer to whatever context is needed by the transport. */
    void* ctx;
};

/**
 * TCP-based transport virtual function table.
 *
 * A set of function pointers representing the TCP functions that must be
 * implemented by every valid transport (see ah_tcp_trans). Every function
 * pointer must set and the function it points to should behave as documented by
 * the function it is named after, unless its documentation dictates otherwise.
 * Each function field takes a void pointer @c ctx as its first argument. This
 * argument is meant to come from the ah_tcp_trans::ctx field of the owning
 * ah_tcp_trans instance. The function pointers are divided into two groups.
 * A given transport will typically only operate on one of these groups. See
 * the field group documentations for more details.
 *
 * @note This structure is primarily useful to those working on TCP transport
 *       implementations.
 */
struct ah_tcp_trans_vtab {
    /**
     * @name TCP Connections
     *
     * Transport operations on ah_tcp_conn instances. If a given transport
     * instance is exclusively associated with one TCP connection via a call to
     * ah_tcp_conn_init(), then you should be able to assume that that instance
     * will only ever be provided to the functions in this group.
     *
     * @{
     */

    ah_err_t (*conn_init)(void* ctx, ah_tcp_conn_t* conn, ah_loop_t* loop, ah_tcp_trans_t trans, ah_tcp_conn_obs_t obs);
    ah_err_t (*conn_open)(void* ctx, ah_tcp_conn_t* conn, const ah_sockaddr_t* laddr);
    ah_err_t (*conn_connect)(void* ctx, ah_tcp_conn_t* conn, const ah_sockaddr_t* raddr);
    ah_err_t (*conn_read_start)(void* ctx, ah_tcp_conn_t* conn);
    ah_err_t (*conn_read_stop)(void* ctx, ah_tcp_conn_t* conn);
    ah_err_t (*conn_write)(void* ctx, ah_tcp_conn_t* conn, ah_tcp_out_t* out);
    ah_err_t (*conn_shutdown)(void* ctx, ah_tcp_conn_t* conn, uint8_t flags);
    ah_err_t (*conn_close)(void* ctx, ah_tcp_conn_t* conn);
    ah_err_t (*conn_term)(void* ctx, ah_tcp_conn_t* conn);
    int (*conn_get_family)(void* ctx, const ah_tcp_conn_t* conn);
    ah_err_t (*conn_get_laddr)(void* ctx, const ah_tcp_conn_t* conn, ah_sockaddr_t* laddr);
    ah_err_t (*conn_get_raddr)(void* ctx, const ah_tcp_conn_t* conn, ah_sockaddr_t* raddr);
    ah_loop_t* (*conn_get_loop)(void* ctx, const ah_tcp_conn_t* conn);
    void* (*conn_get_obs_ctx)(void* ctx, const ah_tcp_conn_t* conn);
    bool (*conn_is_closed)(void* ctx, const ah_tcp_conn_t* conn);
    bool (*conn_is_readable)(void* ctx, const ah_tcp_conn_t* conn);
    bool (*conn_is_reading)(void* ctx, const ah_tcp_conn_t* conn);
    bool (*conn_is_writable)(void* ctx, const ah_tcp_conn_t* conn);
    ah_err_t (*conn_set_keepalive)(void* ctx, ah_tcp_conn_t* conn, bool is_enabled);
    ah_err_t (*conn_set_nodelay)(void* ctx, ah_tcp_conn_t* conn, bool is_enabled);
    ah_err_t (*conn_set_reuseaddr)(void* ctx, ah_tcp_conn_t* conn, bool is_enabled);

    /** @} */

    /**
     * @name TCP Listeners
     *
     * Transport operations on ah_tcp_listener instances. If a given transport
     * instance is exclusively associated with one TCP listener via a call to
     * ah_tcp_listener_init(), then you should be able to assume that that
     * instance will only ever be provided to the functions in this group.
     *
     * @{
     */

    ah_err_t (*listener_init)(void* ctx, ah_tcp_listener_t* ln, ah_loop_t* loop, ah_tcp_trans_t trans, ah_tcp_listener_obs_t obs);
    ah_err_t (*listener_open)(void* ctx, ah_tcp_listener_t* ln, const ah_sockaddr_t* laddr);
    ah_err_t (*listener_listen)(void* ctx, ah_tcp_listener_t* ln, unsigned backlog);
    ah_err_t (*listener_close)(void* ctx, ah_tcp_listener_t* ln);
    ah_err_t (*listener_term)(void* ctx, ah_tcp_listener_t* ln);
    int (*listener_get_family)(void* ctx, const ah_tcp_listener_t* ln);
    ah_err_t (*listener_get_laddr)(void* ctx, const ah_tcp_listener_t* ln, ah_sockaddr_t* laddr);
    ah_loop_t* (*listener_get_loop)(void* ctx, const ah_tcp_listener_t* ln);
    void* (*listener_get_obs_ctx)(void* ctx, const ah_tcp_listener_t* ln);
    bool (*listener_is_closed)(void* ctx, ah_tcp_listener_t* ln);
    ah_err_t (*listener_set_keepalive)(void* ctx, ah_tcp_listener_t* ln, bool is_enabled);
    ah_err_t (*listener_set_nodelay)(void* ctx, ah_tcp_listener_t* ln, bool is_enabled);
    ah_err_t (*listener_set_reuseaddr)(void* ctx, ah_tcp_listener_t* ln, bool is_enabled);

    /**
     * Prepares @a trans for use by a new accepted connection.
     *
     * After this function returns, @a trans @e must have its ah_tcp_trans::vtab
     * field set and @e should have its ah_tcp_trans::ctx field set. If you need
     * to allocate memory or perform some other operation to prepare that
     * context, then this function is where that should be done.
     *
     * @param ctx   Pointer to context.
     * @param ln    Pointer to listener.
     * @param trans Pointer to prepared TCP transport.
     * @return @ref AH_ENONE if successful. If you return any other error code,
     *         the connection being prepared for will be rejected with the same
     *         error.
     *
     * @note This function has no counterpart among the regular functions in
     *       this header file. It exists solely to make it possible for TCP
     *       transports to prepare new transports for connections they accept.
     *
     * @warning If you need to deallocate memory or release other resources when
     *          @a trans is no longer used, do that in
     *          ah_tcp_trans_vtab::conn_term.
     */
    ah_err_t (*listener_prepare)(void* ctx, ah_tcp_listener_t* ln, ah_tcp_trans_t* trans);

    /** @} */
};

/**
 * TCP connection observer.
 *
 * Specifies what functions are to receive events about some ah_tcp_conn
 * instance and what arbitrary context pointer to provide when those functions
 * are invoked.
 */
struct ah_tcp_conn_obs {
    /** Set of connection event callbacks. */
    const ah_tcp_conn_cbs_t* cbs;

    /** Arbitrary pointer provided every time an event callback is fired. */
    void* ctx;
};

/**
 * TCP connection handle.
 *
 * Such a handle can be established either by connecting to a remote listener
 * via ah_tcp_conn_connect() or by accepting connections via
 * ah_tcp_listener_listen().
 *
 * @note All fields of this data structure are @e private in the sense that a
 *       user of this API should not access them directly.
 */
struct ah_tcp_conn {
    AH_I_TCP_CONN_FIELDS
};

/**
 * TCP connection callback set.
 *
 * A set of function pointers used to handle TCP connection events, such as
 * data being received or a connection being opened.
 *
 * Every function takes a context pointer as its first argument. This context
 * pointer comes from the ah_tcp_conn_obs owning this callback set. More
 * specifically, it is a copy of the value of the ah_tcp_conn_obs::ctx field of
 * the owning ah_tcp_conn_obs instance. The context pointer makes it possible
 * for you to associate arbitrary state with individual TCP connections.
 */
struct ah_tcp_conn_cbs {
    /**
     * @a conn has been opened, or the attempt failed.
     *
     * @param ctx  Pointer to context.
     * @param conn Pointer to connection.
     * @param err  @ref AH_ENONE if a connection was opened successfully. What
     *             other error codes are possible depend on the used TCP
     *             transport. The following codes may be provided if the
     *             <em>default transport</em> is used, directly or
     *             indirectly: <ul>
     *   <li>@ref AH_EACCES [Darwin, Linux]         - Not permitted to open connection.
     *   <li>@ref AH_EADDRINUSE                     - Specified local address already in use.
     *   <li>@ref AH_EADDRNOTAVAIL                  - No available local network interface is
     *                                                associated with the given local address.
     *   <li>@ref AH_EAFNOSUPPORT                   - Specified IP version not supported.
     *   <li>@ref AH_ECANCELED                      - Connection event loop is shutting down.
     *   <li>@ref AH_EMFILE [Darwin, Linux, Win32]  - Process descriptor table is full.
     *   <li>@ref AH_ENETDOWN [Win32]               - The network subsystem has failed.
     *   <li>@ref AH_ENFILE [Darwin, Linux]         - System file table is full.
     *   <li>@ref AH_ENOBUFS [Darwin, Linux, Win32] - Not enough buffer space available.
     *   <li>@ref AH_ENOMEM [Darwin, Linux]         - Not enough heap memory available.
     *   <li>@ref AH_EPROVIDERFAILEDINIT [Win32]    - Network service failed to initialize.
     * </ul>
     *
     * @note This function is never called for accepted connections, which
     *       means it may be set to @c NULL when this data structure is used
     *       with ah_tcp_listener_listen().
     * @note Every successfully opened @a conn must eventually be provided to
     *       ah_tcp_conn_close().
     */
    void (*on_open)(void* ctx, ah_tcp_conn_t* conn, ah_err_t err);

    /**
     * @a conn has been established to a specified remote host, or the attempt
     * to establish it has failed.
     *
     * @param ctx  Pointer to context.
     * @param conn Pointer to connection.
     * @param err  @ref AH_ENONE if a connection was established successfully.
     *             What other error codes are possible depend on the used TCP
     *             transport. The following codes may be provided if the
     *             <em>default transport</em> is used, directly or
     *             indirectly: <ul>
     *   <li>@ref AH_EADDRINUSE [Darwin, Linux, Win32] - Failed to bind a concrete local address.
     *                                                   This error only occurs if the connection
     *                                                   was opened with the wildcard address,
     *                                                   which means that network interface
     *                                                   binding is delayed until connection.
     *   <li>@ref AH_EADDRNOTAVAIL [Darwin, Win32]     - The specified remote address is invalid.
     *   <li>@ref AH_EADDRNOTAVAIL [Linux]             - No ephemeral TCP port is available.
     *   <li>@ref AH_EAFNOSUPPORT                      - The IP version of the specified remote
     *                                                   address does not match that of the bound
     *                                                   local address.
     *   <li>@ref AH_ECANCELED                         - The event loop of @a conn has shut down.
     *   <li>@ref AH_ECONNREFUSED                      - Connection attempt ignored or rejected
     *                                                   by targeted remote host.
     *   <li>@ref AH_ECONNRESET [Darwin]               - Connection attempt reset by targeted
     *                                                   remote host.
     *   <li>@ref AH_EHOSTUNREACH                      - The targeted remote host could not be
     *                                                   reached.
     *   <li>@ref AH_ENETDOWN [Darwin]                 - Local network not online.
     *   <li>@ref AH_ENETDOWN [Win32]                  - The network subsystem has failed.
     *   <li>@ref AH_ENETUNREACH                       - Network of targeted remote host not
     *                                                   reachable.
     *   <li>@ref AH_ENOBUFS                           - Not enough buffer space available.
     *   <li>@ref AH_ENOMEM                            - Not enough heap memory available.
     *   <li>@ref AH_ETIMEDOUT                         - The connection attempt did not complete
     *                                                   before its deadline.
     * </ul>
     *
     * @note Data receiving is disabled for new connections by default. It must
     *       be explicitly enabled via a call to ah_tcp_conn_read_start().
     *
     * @note This function is never called for accepted connections, which
     *       means it may be set to @c NULL when this data structure is used
     *       with ah_tcp_listener_listen().
     */
    void (*on_connect)(void* ctx, ah_tcp_conn_t* conn, ah_err_t err);

    /**
     * @a conn has received data from its associated remote host.
     *
     * Successful calls to this function (meaning that @a err is equal to
     * @ref AH_ENONE) always carry a pointer to an ah_tcp_in instance. That
     * instance is reused by @a conn every time this callback is invoked. If
     * the ah_rw field of that instance is not read in its entirety, whatever
     * unread contents remain when this callback returns will be presented
     * again in another call to this callback. If not all of the contents of
     * @a in are read or discarded every time this callback is invoked, or
     * the buffer is repackaged via ah_tcp_in_repackage(), that buffer may
     * eventually become full, triggering the @ref AH_EOVERFLOW error. If you
     * wish to save the contents of @a in without having to copy it over to
     * another buffer, you can detach it from @a conn using ah_tcp_in_detach(),
     * which allocates a new input buffer for @a conn.
     *
     * If this callback is invoked with an error code (@a err is not equal to
     * @ref AH_ENONE), @a conn should always be closed via a call to
     * ah_tcp_conn_close().
     *
     * @param ctx  Pointer to context.
     * @param conn Pointer to connection.
     * @param in   Pointer to input data representation, or @c NULL if @a err
     *             is not @ref AH_ENONE.
     * @param err  @ref AH_ENONE if a data was received successfully. What other
     *             error codes are possible depend on the used TCP transport.
     *             The following codes may be provided if the <em>default
     *             transport</em> is used, directly or indirectly: <ul>
     *   <li>@ref AH_ECANCELED                  - Connection event loop is shutting down.
     *   <li>@ref AH_ECONNABORTED [Win32]       - Virtual circuit terminated due to time-out or
     *                                            other failure.
     *   <li>@ref AH_ECONNRESET [Darwin, Win32] - Connection reset by remote host.
     *   <li>@ref AH_EDISCON [Win32]            - Connection gracefully closed by remote host.
     *   <li>@ref AH_EEOF                       - Connection closed for reading.
     *   <li>@ref AH_ENETDOWN [Win32]           - The network subsystem has failed.
     *   <li>@ref AH_ENETRESET [Win32]          - Keep-alive is enabled for the connection and a
     *                                            related failure was detected.
     *   <li>@ref AH_ENOBUFS [Darwin, Linux]    - Not enough buffer space available.
     *   <li>@ref AH_ENOMEM [Linux]             - Not enough heap memory available.
     *   <li>@ref AH_EOVERFLOW                  - The input buffer of @a conn is full. Note that
     *                                            the input buffer is not available via @a in if
     *                                            this error code is provided. The only way to
     *                                            recover from this error is by closing the
     *                                            connection. To prevent this error from
     *                                            occurring, you must ensure that the input
     *                                            buffer never gets exhausted by reading,
     *                                            discarding, repackaging or detaching
     *                                            its contents, as described further above.
     *   <li>@ref AH_ETIMEDOUT                  - Connection timed out.
     * </ul>
     */
    void (*on_read)(void* ctx, ah_tcp_conn_t* conn, ah_tcp_in_t* in, ah_err_t err);

    /**
     * @a conn has sent data to its associated remote host.
     *
     * This callback is always invoked after a successful call to
     * ah_tcp_conn_write(). If @a err is @ref AH_ENONE, all outgoing data
     * provided to the mentioned function was transmitted successfully. If @a
     * err has any other value, an error occurred before the transmission could
     * be completed. If an error has occurred, @a conn should be closed using
     * ah_tcp_conn_close().
     *
     * @param ctx  Pointer to context.
     * @param conn Pointer to connection.
     * @param out  Pointer to output buffer provided to ah_tcp_conn_write(). If
     *             @a err is not @ref AH_ENONE, it @e may, but should never, be
     *             @c NULL.
     * @param err  @ref AH_ENONE if the data was sent successfully. What other
     *             error codes are possible depend on the used TCP transport.
     *             The following codes may be provided if the <em>default
     *             transport</em> is used, directly or indirectly: <ul>
     *   <li>@ref AH_ECANCELED                         - Connection event loop is shutting down.
     *   <li>@ref AH_ECONNABORTED [Win32]              - Virtual circuit terminated due to time-out
     *                                                   or other failure.
     *   <li>@ref AH_ECONNRESET [Darwin, Linux, Win32] - Connection reset by remote host.
     *   <li>@ref AH_EEOF                              - Connection closed for writing.
     *   <li>@ref AH_ENETDOWN [Darwin]                 - Local network not online.
     *   <li>@ref AH_ENETDOWN [Win32]                  - The network subsystem has failed.
     *   <li>@ref AH_ENETRESET [Win32]                 - Keep-alive is enabled for the connection
     *                                                   and a related failure was detected.
     *   <li>@ref AH_ENETUNREACH [Darwin]              - Network of remote host not reachable.
     *   <li>@ref AH_ENOBUFS [Darwin, Linux, Win32]    - Not enough buffer space available.
     *   <li>@ref AH_ENOMEM [Darwin, Linux]            - Not enough heap memory available.
     *   <li>@ref AH_ETIMEDOUT                         - Connection timed out.
     * </ul>
     */
    void (*on_write)(void* ctx, ah_tcp_conn_t* conn, ah_tcp_out_t* out, ah_err_t err);

    /**
     * @a conn has been closed.
     *
     * @param ctx  Pointer to context.
     * @param conn Pointer to connection.
     * @param err  Should always be @ref AH_ENONE. Other codes may be provided if
     *             an unexpected platform error occurs.
     *
     * @note This function is guaranteed to be called after every call to
     *       ah_tcp_conn_close(), which makes it an excellent place to release
     *       any resources associated with @a conn.
     */
    void (*on_close)(void* ctx, ah_tcp_conn_t* conn, ah_err_t err);
};

/**
 * TCP listener observer.
 *
 * Specifies what functions are to receive events about some ah_tcp_listener
 * instance and what user pointer to provide to those functions.
 */
struct ah_tcp_listener_obs {
    /** Set of listener event callbacks. */
    const ah_tcp_listener_cbs_t* cbs;

    /** Arbitrary pointer provided every time an event callback is fired. */
    void* ctx;
};

/**
 * TCP listener handle.
 *
 * Such a handle may represent the attempt to accept incoming TCP connections.
 *
 * @note All fields of this data structure are @e private in the sense that a
 *       user of this API should not access them directly.
 */
struct ah_tcp_listener {
    AH_I_TCP_LISTENER_FIELDS
};

/**
 * TCP listener callback set.
 *
 * A set of function pointers used to handle TCP listener events, such as a
 * listener being opened or a connection being accepted.
 *
 * Every function takes a context pointer as its first argument. This context
 * pointer comes from the ah_tcp_listener_obs owning this callback set. More
 * specifically, it is a copy of the value of the ah_tcp_listener_obs::ctx field
 * of the owning ah_tcp_listener_obs instance. The context pointer makes it
 * possible for you to associate arbitrary state with individual TCP listeners.
 */
struct ah_tcp_listener_cbs {
    /**
     * @a ln has been opened, or the attempt failed.
     *
     * @param ctx Pointer to context.
     * @param ln  Pointer to listener.
     * @param err @ref AH_ENONE if @a ln was opened successfully. What other
     *            error codes are possible depend on the used TCP transport.
     *            The following codes may be provided if the <em>default
     *            transport</em> is used, directly or indirectly: <ul>
     *   <li>@ref AH_EACCES [Darwin, Linux]         - Not permitted to open listener.
     *   <li>@ref AH_EADDRINUSE                     - Specified local address already in use.
     *   <li>@ref AH_EADDRNOTAVAIL                  - No available local network interface is
     *                                                associated with the given local address.
     *   <li>@ref AH_EAFNOSUPPORT                   - Specified IP version not supported.
     *   <li>@ref AH_ECANCELED                      - Connection event loop is shutting down.
     *   <li>@ref AH_EMFILE [Darwin, Linux, Win32]  - Process descriptor table is full.
     *   <li>@ref AH_ENETDOWN [Win32]               - The network subsystem has failed.
     *   <li>@ref AH_ENFILE [Darwin, Linux]         - System file table is full.
     *   <li>@ref AH_ENOBUFS [Darwin, Linux, Win32] - Not enough buffer space available.
     *   <li>@ref AH_ENOMEM [Darwin, Linux]         - Not enough heap memory available.
     * </ul>
     */
    void (*on_open)(void* ctx, ah_tcp_listener_t* ln, ah_err_t err);

    /**
     * @a ln has started to listen for incoming connections, or the attempt
     * failed.
     *
     * @param ctx  Pointer to context.
     * @param ln   Pointer to listener.
     * @param err  @ref AH_ENONE if @a ln started to listen successfully. What
     *             other error codes are possible depend on the used TCP
     *             transport. The following codes may be provided if the
     *             <em>default transport</em> is used, directly or
     *             indirectly: <ul>
     *   <li>@ref AH_EACCES [Darwin]           - Not permitted to listen.
     *   <li>@ref AH_EADDRINUSE [Linux, Win32] - No ephemeral TCP port is available. This error
     *                                           can only occur if the listener was opened with
     *                                           the wildcard address, which means that network
     *                                           interface binding is delayed until listening.
     *   <li>@ref AH_ECANCELED                 - Listener event loop is shutting down.
     *   <li>@ref AH_ENETDOWN [Win32]          - The network subsystem has failed.
     *   <li>@ref AH_ENFILE [Win32]            - System file table is full.
     *   <li>@ref AH_ENOBUFS [Win32]           - Not enough buffer space available.
     * </ul>
     */
    void (*on_listen)(void* ctx, ah_tcp_listener_t* ln, ah_err_t err);

    /**
     * @a ln has accepted the connection in @a accept.
     *
     * If @a err is @ref AH_ENONE, which indicates a successful acceptance, you
     * must <ol>
     *   <li>set the connection observer (ah_tcp_accept::obs) in @a accept, and
     *   <li>call ah_tcp_conn_close() with the connection (ah_tcp_accept::conn)
     *       in @a accept once it is no longer in use.
     * </ol>
     * If you close the connection already in this callback, setting the
     * connection observer in @a accept becomes unnecessary. If you do not close
     * the connection and also do not set the connection observer in @a accept,
     * this callback will be invoked again with @a err set to @ref AH_ESTATE.
     *
     * The remote address pointer (ah_tcp_accept::raddr) in @a accept, as well
     * as the @a accept instance itself, are only guaranteed to exist until this
     * callback returns. You may copy the remote address somewhere else to store
     * it, or you may call ah_tcp_conn_get_raddr() at some later point to
     * retrieve it again.
     *
     * @param ctx    Pointer to context.
     * @param ln     Pointer to listener.
     * @param accept Pointer to structure containing pointer to the accepted
     *               connection and other associated data, or @c NULL if @a err
     *               is not @ref AH_ENONE.
     * @param err    @ref AH_ENONE if @a ln accepted connection successfully.
     *               What other error codes are possible depend on the used TCP
     *               transport. The following codes may be provided if the
     *               <em>default transport</em> is used, directly or
     *               indirectly: <ul>
     *   <li>@ref AH_ECANCELED                     - Listener event loop is shutting down.
     *   <li>@ref AH_ECONNABORTED [Darwin, Linux]  - Connection aborted before finalization.
     *   <li>@ref AH_ECONNRESET [Win32]            - Connection aborted before finalization.
     *   <li>@ref AH_EMFILE [Darwin, Linux, Win32] - Process descriptor table is full.
     *   <li>@ref AH_ENETDOWN [Win32]              - The network subsystem has failed.
     *   <li>@ref AH_ENFILE [Darwin, Linux]        - System file table is full.
     *   <li>@ref AH_ENOBUFS [Linux, Win32]        - Not enough buffer space available.
     *   <li>@ref AH_ENOMEM [Darwin, Linux]        - Not enough heap memory available.
     *   <li>@ref AH_EPROVIDERFAILEDINIT [Win32]   - Network service failed to initialize.
     *   <li>@ref AH_ESTATE                        - @a obs not set in a previous call to this
     *                                               function, as explained above.
     * </ul>
     *
     * @note Every successfully accepted TCP connection must eventually be
     *       provided to ah_tcp_conn_close().
     *
     * @note Data receiving is disabled for accepted connections by default. It
     *       must be explicitly enabled via a call to ah_tcp_conn_read_start().
     */
    void (*on_accept)(void* ctx, ah_tcp_listener_t* ln, ah_tcp_accept_t* accept, ah_err_t err);

    /**
     * @a ln has been closed.
     *
     * @param ctx  Pointer to context.
     * @param conn Pointer to listener.
     * @param err  Should always be @ref AH_ENONE. Other codes may be provided if
     *             an unexpected platform error occurs.
     *
     * @note This function is guaranteed to be called after every call to
     *       ah_tcp_listener_close(), which makes it an excellent place to
     *       release any resources associated with @a ln. You may, for example,
     *       elect to call ah_tcp_listener_term() in this callback.
     */
    void (*on_close)(void* ctx, ah_tcp_listener_t* ln, ah_err_t err);
};

/**
 * TCP listener acceptance.
 *
 * Represents an incoming TCP connection, and other associated data, accepted
 * by a TCL listener.
 */
struct ah_tcp_accept {
    void* ctx;                  ///< TCP transport context associated with @a conn, or @c NULL if none.
    ah_tcp_conn_t* conn;        ///< Pointer to accepted TCP connection.
    ah_tcp_conn_obs_t* obs;     ///< Pointer to event observer of @a conn.
    const ah_sockaddr_t* raddr; ///< Pointer to remote address of @a conn.
};

/**
 * TCP input stream.
 *
 * @note Some fields of this data structure are @e private in the sense that a
 *       user of this API should not access them directly. All private fields
 *       have names beginning with an underscore.
 */
struct ah_tcp_in {
    /** Reader/writer referring to incoming data. */
    ah_rw_t rw;

    AH_I_TCP_IN_FIELDS
};

/**
 * TCP output buffer.
 *
 * @note Some fields of this data structure are @e private in the sense that a
 *       user of this API should not access them directly. All private fields
 *       have names beginning with an underscore.
 */
struct ah_tcp_out {
    /** Buffer referring to outgoing data. */
    ah_buf_t buf;

    AH_I_TCP_OUT_FIELDS
};

/**
 * @name TCP Connections
 *
 * Operations on ah_tcp_conn instances. All such instances must be initialized
 * using ah_tcp_conn_init() before they are provided to any other functions
 * listed here. Any other requirements regarding the state of connections
 * are described in the documentation of each respective function, sometimes
 * only via the error codes it lists.
 *
 * @{
 */

/**
 * Initializes @a conn for subsequent use.
 *
 * @param conn  Pointer to connection.
 * @param loop  Pointer to event loop.
 * @param trans Desired transport.
 * @param obs   Pointer to event callback set.
 * @return One of the following error codes: <ul>
 *   <li>@ref AH_ENONE  - Operation successful.
 *   <li>@ref AH_EINVAL - @a conn is @c NULL, @a trans @c ->vtab is @c NULL or @a trans
 *                        @c ->vtab->conn_init is @c NULL.
 *   <li>Any additional code returned by the used TCP transport.
 * </ul>
 * The <em>default transport</em> may also cause any of the following error
 * codes to be returned: <ul>
 *   <li>@ref AH_EINVAL - @a loop or @a cbs is @c NULL.
 *   <li>@ref AH_EINVAL - @a trans @c vtab is invalid, as reported by ah_tcp_trans_vtab_is_valid().
 *   <li>@ref AH_EINVAL - @a obs @c cbs is invalid, as reported by
 *                        ah_tcp_conn_cbs_is_valid_for_connection().
 * </ul>
 *
 * @note Every successfully initialized @a conn must eventually be provided to
 *       ah_tcp_conn_term(). Normally, this is done in two places: <ol>
 *         <li>in the ah_tcp_conn_cbs::on_open callback of @a conn after a check that its @c err
 *             argument is not @ref AH_ENONE, and
 *         <li>unconditionally in the ah_tcp_conn_cbs::on_close callback.
 * </ol>
 */
ah_extern ah_err_t ah_tcp_conn_init(ah_tcp_conn_t* conn, ah_loop_t* loop, ah_tcp_trans_t trans, ah_tcp_conn_obs_t obs);

/**
 * Schedules opening of @a conn, which must be initialized, and its binding to
 * the local network interface represented by @a laddr.
 *
 * If the return value of this function is @ref AH_ENONE, meaning that the open
 * attempt could indeed be scheduled, its result will eventually be presented
 * via the ah_tcp_conn_cbs::on_open callback of @a conn.
 *
 * @param conn  Pointer to connection.
 * @param laddr Pointer to socket address representing a local network
 *              interface through which the connection must later be
 *              established. If opening is successful, the referenced address
 *              must remain valid for the entire lifetime of the created
 *              connection. To bind to all or any local network interface,
 *              provide the wildcard address (see ah_sockaddr_ipv4_wildcard and
 *              ah_sockaddr_ipv6_wildcard). If you want the platform to chose
 *              port number automatically, specify port @c 0.
 * @return One of the following error codes: <ul>
 *   <li>@ref AH_ENONE        - Operation successful.
 *   <li>@ref AH_EINVAL       - @a conn is @c NULL, @a conn @c ->vtab is @c NULL or @a conn
 *                              @c ->vtab->conn_open is @c NULL.
 *   <li>Any additional code returned by the used TCP transport.
 * </ul>
 * The <em>default transport</em> may also cause any of the following error
 * codes to be returned: <ul>
 *   <li>@ref AH_EAFNOSUPPORT - @a laddr is not an IP-based address.
 *   <li>@ref AH_ECANCELED    - The event loop of @a conn is shutting down.
 *   <li>@ref AH_EINVAL       - @a laddr is @c NULL.
 *   <li>@ref AH_ENOBUFS      - Not enough buffer space available.
 *   <li>@ref AH_ENOMEM       - Not enough heap memory available.
 *   <li>@ref AH_ESTATE       - @a conn can be determined not to have been properly initialized.
 * </ul>
 *
 * @note Every successfully opened @a conn must eventually be provided to
 *       ah_tcp_conn_close().
 *
 * @warning This function must be called with a successfully initialized
 *           connection.
 */
ah_extern ah_err_t ah_tcp_conn_open(ah_tcp_conn_t* conn, const ah_sockaddr_t* laddr);

/**
 * Schedules connection of @a conn, which must be open, to @a raddr.
 *
 * If the return value of this function is @ref AH_ENONE, meaning that
 * connection could indeed be scheduled, its result will eventually be presented
 * via the ah_tcp_conn_cbs::on_connect callback of @a conn.
 *
 * @param conn  Pointer to connection.
 * @param raddr Pointer to socket address representing the remote host to which
 *              the connection is to be established. If connection is
 *              successful, the referenced address must remain valid until
 *              @a conn is closed.
 * @return One of the following error codes: <ul>
 *   <li>@ref AH_ENONE        - Operation successful.
 *   <li>@ref AH_EINVAL       - @a conn is @c NULL, @a conn @c ->vtab is @c NULL or @a conn
 *                              @c ->vtab->conn_connect is @c NULL.
 *   <li>Any additional code returned by the used TCP transport.
 * </ul>
 * The <em>default transport</em> may also cause any of the following error
 * codes to be returned: <ul>
 *   <li>@ref AH_EAFNOSUPPORT - @a raddr is not an IP-based address.
 *   <li>@ref AH_ECANCELED    - The event loop of @a conn is shutting down.
 *   <li>@ref AH_EINVAL       - @a conn or @a raddr is @c NULL.
 *   <li>@ref AH_ENOBUFS      - Not enough buffer space available.
 *   <li>@ref AH_ENOMEM       - Not enough heap memory available.
 *   <li>@ref AH_ESTATE       - @a conn is not open.
 * </ul>
 *
 * @note Data receiving is disabled for new connections by default. Is must be
 *       explicitly enabled via a call to ah_tcp_conn_read_start().
 *
 * @warning This function must be called with a successfully opened connection.
 *          An appropriate place to call this function is often going to be in
 *          an ah_tcp_conn_cbs::on_open callback after a check that opening was
 *          successful.
 */
ah_extern ah_err_t ah_tcp_conn_connect(ah_tcp_conn_t* conn, const ah_sockaddr_t* raddr);

/**
 * Enables receiving of incoming data via @a conn.
 *
 * When the receiving of data is enabled, the ah_tcp_conn_cbs::on_read callback
 * of @a conn will be invoked whenever incoming data is received.
 *
 * @param conn Pointer to connection.
 * @return One of the following error codes: <ul>
 *   <li>@ref AH_ENONE            - Operation successful.
 *   <li>@ref AH_EINVAL           - @a conn is @c NULL, @a conn @c ->vtab is @c NULL or @a conn
 *                                  @c ->vtab->conn_read_start is @c NULL.
 *   <li>Any additional code returned by the used TCP transport.
 * </ul>
 * The <em>default transport</em> may also cause any of the following error
 * codes to be returned: <ul>
 *   <li>@ref AH_ECANCELED        - The event loop of @a conn is shutting down.
 *   <li>@ref AH_ENETDOWN [Win32] - The network subsystem has failed.
 *   <li>@ref AH_ENOBUFS          - Not enough buffer space available.
 *   <li>@ref AH_ENOMEM           - Not enough heap memory available.
 *   <li>@ref AH_EOVERFLOW        - @c AH_PSIZE is too small for it to be possible to store both
 *                                  required metadata @e and read data in a single page provided
 *                                  by the page allocator (see ah_palloc()).
 *   <li>@ref AH_ESTATE           - @a conn is not connected or its read direction is shut down.
 * </ul>
 *
 * @warning This function must be called with a successfully connected
 *          connection. An appropriate place to call this function is often
 *          going to be in an ah_tcp_conn_cbs::on_connect callback after a
 *          check that the connection attempt was successful.
 */
ah_extern ah_err_t ah_tcp_conn_read_start(ah_tcp_conn_t* conn);

/**
 * Disables receiving of incoming data via @a conn.
 *
 * @param conn Pointer to connection.
 * @return One of the following error codes: <ul>
 *   <li>@ref AH_ENONE  - Operation successful.
 *   <li>@ref AH_EINVAL - @a conn is @c NULL, @a conn @c ->vtab is @c NULL or @a conn
 *                        @c ->vtab->conn_read_stop is @c NULL.
 *   <li>Any additional code returned by the used TCP transport.
 * </ul>
 * The <em>default transport</em> may also cause the following error code to be
 * returned: <ul>
 *   <li>@ref AH_ESTATE - @a conn reading not started.
 * </ul>
 *
 * @note It is acceptable to call this function immediately after a successful
 *       call to ah_tcp_conn_read_start() with the same @a conn, even if that
 *       means that @a conn never had a practical chance to start reading.
 *
 * @warning This function must be called with a connection that has successfully
 *          started reading.
 */
ah_extern ah_err_t ah_tcp_conn_read_stop(ah_tcp_conn_t* conn);

/**
 * Schedules the sending of the data in @a out to the remote host of @a conn.
 *
 * An output buffer can be allocated on the heap using ah_tcp_out_alloc(). If
 * you want to store the buffer memory somewhere else, just zero an ah_tcp_out
 * instance and then initialize its @c buf field.
 *
 * If the return value of this function is @ref AH_ENONE, meaning that the
 * sending could indeed be scheduled, the result of the sending will eventually
 * be presented via the ah_tcp_conn_cbs::on_write callback of @a conn. More
 * specifically, the callback is invoked either if an error occurs or after all
 * data in @a out has been successfully transmitted.
 *
 * @param conn Pointer to connection.
 * @param out  Pointer to outgoing data.
 * @return One of the following error codes: <ul>
 *   <li>@ref AH_ENONE            - Operation successful.
 *   <li>@ref AH_EINVAL           - @a conn is @c NULL, @a conn @c ->vtab is @c NULL or @a conn
 *                                  @c ->vtab->conn_write is @c NULL.
 *   <li>Any additional code returned by the used TCP transport.
 * </ul>
 * The <em>default transport</em> may also cause any of the following error
 * codes to be returned: <ul>
 *   <li>@ref AH_ECANCELED        - The event loop of @a conn is shutting down.
 *   <li>@ref AH_EINVAL           - out is @c NULL.
 *   <li>@ref AH_ENETDOWN [Win32] - The network subsystem has failed.
 *   <li>@ref AH_ENOBUFS          - Not enough buffer space available.
 *   <li>@ref AH_ENOMEM           - Not enough heap memory available.
 *   <li>@ref AH_ESTATE           - @a conn is not connected or its write direction is shut down.
 * </ul>
 *
 * @warning This function must be called with a successfully connected
 *          connection.
 */
ah_extern ah_err_t ah_tcp_conn_write(ah_tcp_conn_t* conn, ah_tcp_out_t* out);

/**
 * Shuts down the read and/or write direction of @a conn, as specified by
 * @a flags.
 *
 * When and how the read and/or write directions of @a conn are shut down varies
 * with the underlying platform. No guarantees are given about how fast the
 * platform will release any resources associated with reading or writing. What
 * is guaranteed, however, is that further reads and writes you issue via
 * @a conn will be rejected and that the results of any on-going reads and/or
 * writes will be silently discarded.
 *
 * @param conn  Pointer to connection.
 * @param flags Shutdown flags.
 * @return One of the following error codes: <ul>
 *   <li>@ref AH_ENONE                - Operation successful.
 *   <li>@ref AH_EINVAL               - @a conn is @c NULL, @a conn @c ->vtab is @c NULL or @a conn
 *                                      @c ->vtab->conn_shutdown is @c NULL.
 *   <li>Any additional code returned by the used TCP transport.
 * </ul>
 * The <em>default transport</em> may also cause any of the following error
 * codes to be returned: <ul>
 *   <li>@ref AH_ECONNABORTED [Win32] - Connection has been aborted.
 *   <li>@ref AH_ECONNRESET [Win32]   - Connection has been reset by its remote host.
 *   <li>@ref AH_ENETDOWN [Win32]     - The network subsystem has failed.
 *   <li>@ref AH_ESTATE               - @a conn is not connected.
 * </ul>
 *
 * @warning A connection with both of its read and write directions shut down
 *          is not considered as being closed. Every connection must eventually
 *          be provided to ah_tcp_conn_close(), irrespective of any direction
 *          being shutdown.
 */
ah_extern ah_err_t ah_tcp_conn_shutdown(ah_tcp_conn_t* conn, uint8_t flags);

/**
 * Schedules closing of @a conn.
 *
 * If the return value of this function is @ref AH_ENONE, meaning that the
 * closing could indeed be scheduled, its result will eventually be presented
 * via the ah_tcp_conn_cbs::on_close callback of @a conn.
 *
 * @param conn Pointer to connection.
 * @return One of the following error codes: <ul>
 *   <li>@ref AH_ENONE  - Operation successful.
 *   <li>@ref AH_EINVAL - @a conn is @c NULL, @a conn @c ->vtab is @c NULL or @a conn
 *                        @c ->vtab->conn_close is @c NULL.
 * </ul>
 * The <em>default transport</em> may also cause the following error code to be
 * returned: <ul>
 *   <li>@ref AH_ESTATE - @a conn is not open.
 * </ul>
 *
 * @warning This function must be called with a successfully opened connection.
 */
ah_extern ah_err_t ah_tcp_conn_close(ah_tcp_conn_t* conn);

/**
 * Terminates @a conn, releasing any resources it may hold.
 *
 * @param conn Pointer to connection.
 * @return One of the following error codes: <ul>
 *   <li>@ref AH_ENONE  - Operation successful.
 *   <li>@ref AH_EINVAL - @a conn is @c NULL, @a conn @c ->vtab is @c NULL or @a conn
 *                        @c ->vtab->conn_term is @c NULL.
 * </ul>
 * The <em>default transport</em> may also cause the following error code to be
 * returned: <ul>
 *   <li>@ref AH_ESTATE - @a conn is not closed.
 * </ul>
 *
 * @warning This function must be called with a successfully closed connection.
 */
ah_extern ah_err_t ah_tcp_conn_term(ah_tcp_conn_t* conn);

/**
 * Checks the socket family of @a conn.
 *
 * For most TCP transports you may expect this socket family to be the same as
 * was specified in the call to ah_tcp_conn_open() through which @a conn was
 * first opened.
 *
 * @param conn Pointer to connection.
 * @return Socket family identifier or @c -1 if @a conn is @c NULL or some other
 *         error occurred. If the <em>default transport</em> is used, the
 *         following identifiers may be produced: <ul>
 *   <li>@ref AH_SOCKFAMILY_IPV4 - IPv4 family identifier.
 *   <li>@ref AH_SOCKFAMILY_IPV6 - IPv6 family identifier.
 * </ul>
 */
ah_extern int ah_tcp_conn_get_family(const ah_tcp_conn_t* conn);

/**
 * Stores local address bound by @a conn into @a laddr.
 *
 * If @a conn was opened with a zero port, this function @e should report what
 * concrete port was assigned to @a conn. If the <em>default transport</em>
 * is used and this call is not intercepted by an intermediary transport, this
 * function @e will report the assigned port.
 *
 * @param conn  Pointer to connection.
 * @param laddr Pointer to socket address to be set by this operation.
 * @return One of the following error codes: <ul>
 *   <li>@ref AH_ENONE                   - The operation was successful.
 *   <li>@ref AH_EINVAL                  - @a conn is @c NULL, @a conn @c ->vtab is @c NULL or
 *                                         @a conn @c ->vtab->conn_get_laddr is @c NULL.
 * </ul>
 * The <em>default transport</em> may also cause any of the following error
 * codes to be returned: <ul>
 *   <li>@ref AH_EINVAL                  - @a laddr is @c NULL.
 *   <li>@ref AH_ENETDOWN [Win32]        - The network subsystem has failed.
 *   <li>@ref AH_ENOBUFS [Darwin, Linux] - Not enough buffer space available.
 *   <li>@ref AH_ESTATE                  - @a conn is closed.
 * </ul>
 */
ah_extern ah_err_t ah_tcp_conn_get_laddr(const ah_tcp_conn_t* conn, ah_sockaddr_t* laddr);

/**
 * Stores remote address of @a conn into @a raddr.
 *
 * @param conn  Pointer to connection.
 * @param raddr Pointer to socket address to be set by this operation.
 * @return One of the following error codes: <ul>
 *   <li>@ref AH_ENONE                   - The operation was successful.
 *   <li>@ref AH_EINVAL                  - @a conn is @c NULL, @a conn @c ->vtab is @c NULL or
 *                                         @a conn @c ->vtab->conn_get_raddr is @c NULL.
 * </ul>
 * The <em>default transport</em> may also cause any of the following error
 * codes to be returned: <ul>
 *   <li>@ref AH_EINVAL                  - @a raddr is @c NULL.
 *   <li>@ref AH_ENETDOWN [Win32]        - The network subsystem has failed.
 *   <li>@ref AH_ENOBUFS [Darwin, Linux] - Not enough buffer space available.
 *   <li>@ref AH_ESTATE                  - @a conn is not connected to a remote host.
 * </ul>
 */
ah_extern ah_err_t ah_tcp_conn_get_raddr(const ah_tcp_conn_t* conn, ah_sockaddr_t* raddr);

/**
 * Gets pointer to event loop of @a conn.
 *
 * This event loop @e should be the same one as was provided when @a conn was
 * first initialized via a call to ah_tcp_conn_init(). If the <em>default
 * transport</em> is used and this function is not intercepted by an
 * intermediary transport, this function @e will return the mentioned original
 * event loop pointer.
 *
 * @param conn Pointer to connection.
 * @return Loop pointer, or @c NULL if @a conn is @c NULL, @a conn @c ->vtab
 *         is @c NULL or @a conn @c ->vtab->conn_get_loop is @c NULL. Also
 *         returns @c NULL if the loop pointer itself is equal to @c NULL.
 */
ah_extern ah_loop_t* ah_tcp_conn_get_loop(const ah_tcp_conn_t* conn);

/**
 * Gets the context pointer of the connection observer associated with @a conn.
 *
 * @param conn Pointer to connection.
 * @return Context pointer, or @c NULL if @a conn is @c NULL, @a conn @c ->vtab
 *         is @c NULL or @a conn @c ->vtab->conn_get_obs_ctx is @c NULL. Also
 *         returns @c NULL if the context pointer itself is equal to @c NULL.
 */
ah_extern void* ah_tcp_conn_get_obs_ctx(const ah_tcp_conn_t* conn);

/**
 * Checks if @a conn is in any closing or closed state.
 *
 * Also newly initialized and terminated connections are considered closed.
 *
 * @param conn Pointer to connection.
 * @return @c true only if @a conn is currently closing or closed. @c false if
 *         @a conn is @c NULL, @a conn @c ->vtab is @c NULL or @a conn
 *         @c ->vtab->conn_is_closed is @c NULL.
 */
ah_extern bool ah_tcp_conn_is_closed(const ah_tcp_conn_t* conn);

/**
 * Checks if @a conn can be read from.
 *
 * A readable connection is currently connected and has not had its read
 * direction shut down.
 *
 * @param conn Pointer to connection.
 * @return @c true only if @a conn is currently readable. @c false if @a conn
 *         is @c NULL, @a conn @c ->vtab is @c NULL or @a conn
 *         @c ->vtab->conn_is_readable is @c NULL.
 */
ah_extern bool ah_tcp_conn_is_readable(const ah_tcp_conn_t* conn);

/**
 * Checks if @a conn is currently reading incoming data.
 *
 * A connection is reading if its currently connected and
 * ah_tcp_conn_read_start() has been called with the same connection as
 * argument. In addition, neither of ah_tcp_conn_read_stop() or
 * ah_tcp_conn_shutdown() has since been used to stop or shutdown its read
 * direction.
 *
 * @param conn Pointer to connection.
 * @return @c true only if @a conn is currently reading, as defined above.
 *         @c false if @a conn is @c NULL, @a conn @c ->vtab is @c NULL or
 *         @a conn @c ->vtab->conn_is_reading is @c NULL.
 */
ah_extern bool ah_tcp_conn_is_reading(const ah_tcp_conn_t* conn);

/**
 * Checks if @a conn can be written to.
 *
 * A writable connection is currently connected and has not had its write
 * direction shut down.
 *
 * @param conn Pointer to connection.
 * @return @c true only if @a conn is currently writable. @c false if @a conn
 *         is @c NULL, @a conn @c ->vtab is @c NULL or @a conn
 *         @c ->vtab->conn_is_writable is @c NULL.
 */
ah_extern bool ah_tcp_conn_is_writable(const ah_tcp_conn_t* conn);

/**
 * Sets the @e keep-alive option of @a conn to @a is_enabled.
 *
 * This option enables or disables keep-alive messaging. Generally, using such
 * messaging means that @a conn automatically sends messages at sensible times
 * to check if the connection is in a usable condition. The exact implications
 * of this option depends on the platform.
 *
 * @param conn       Pointer to connection.
 * @param is_enabled Whether keep-alive is to be enabled or not.
 * @return One of the following error codes: <ul>
 *   <li>@ref AH_ENONE            - The operation was successful.
 *   <li>@ref AH_EINVAL           - @a conn is @c NULL, @a conn @c ->vtab is @c NULL or @a conn
 *                                  @c ->vtab->conn_set_keepalive is @c NULL.
 * </ul>
 * The <em>default transport</em> may also cause any of the following error
 * codes to be returned: <ul>
 *   <li>@ref AH_ENETDOWN [Win32] - The network subsystem has failed.
 *   <li>@ref AH_ENOBUFS [Darwin] - Not enough buffer space available.
 *   <li>@ref AH_ENOMEM [Darwin]  - Not enough heap memory available.
 *   <li>@ref AH_ESTATE           - @a conn is closed.
 * </ul>
 */
ah_extern ah_err_t ah_tcp_conn_set_keepalive(ah_tcp_conn_t* conn, bool is_enabled);

/**
 * Sets the @e no-delay option of @a conn to @a is_enabled.
 *
 * This option being enabled means that use of Nagle's algorithm is disabled.
 * The mentioned algorithm queues up messages for a short time before sending
 * them over the network. The purpose of this is to reduce the number of TCP
 * segments submitted over the used network. Its disadvantage is that it may
 * increase messaging latency.
 *
 * @param conn       Pointer to connection.
 * @param is_enabled Whether keep-alive is to be enabled or not.
 * @return One of the following error codes: <ul>
 *   <li>@ref AH_ENONE            - The operation was successful.
 *   <li>@ref AH_EINVAL           - @a conn is @c NULL, @a conn @c ->vtab is @c NULL or @a conn
 *                                  @c ->vtab->conn_set_nodelay is @c NULL.
 * </ul>
 * The <em>default transport</em> may also cause any of the following error
 * codes to be returned: <ul>
 *   <li>@ref AH_ENETDOWN [Win32] - The network subsystem has failed.
 *   <li>@ref AH_ENOBUFS [Darwin] - Not enough buffer space available.
 *   <li>@ref AH_ENOMEM [Darwin]  - Not enough heap memory available.
 *   <li>@ref AH_ESTATE           - @a conn is closed.
 * </ul>
 */
ah_extern ah_err_t ah_tcp_conn_set_nodelay(ah_tcp_conn_t* conn, bool is_enabled);

/**
 * Sets the <em>reuse address</em> option of @a conn to @a is_enabled.
 *
 * Address reuse generally means that a the specific combination of local
 * interface address and port number bound by this connection can be reused
 * right after it closes. Address reuse can lead to security implications as
 * it may enable a malicious process on the same platform to hijack a closed
 * connection.
 *
 * @param conn       Pointer to connection.
 * @param is_enabled Whether keep-alive is to be enabled or not.
 * @return One of the following error codes: <ul>
 *   <li>@ref AH_ENONE            - The operation was successful.
 *   <li>@ref AH_EINVAL           - @a conn is @c NULL, @a conn @c ->vtab is @c NULL or @a conn
 *                                  @c ->vtab->conn_set_reuseaddr is @c NULL.
 * </ul>
 * The <em>default transport</em> may also cause any of the following error
 * codes to be returned: <ul>
 *   <li>@ref AH_ENETDOWN [Win32] - The network subsystem has failed.
 *   <li>@ref AH_ENOBUFS [Darwin] - Not enough buffer space available.
 *   <li>@ref AH_ENOMEM [Darwin]  - Not enough heap memory available.
 *   <li>@ref AH_ESTATE           - @a conn is closed.
 * </ul>
 */
ah_extern ah_err_t ah_tcp_conn_set_reuseaddr(ah_tcp_conn_t* conn, bool is_enabled);

/** @} */

/**
 * @name TCP Connection Callback Sets
 *
 * Operations on ah_tcp_conn_cbs instances.
 *
 * @{
 */

/**
 * Checks if @a cbs is valid for handling connections accepted by a TCP
 * listener.
 *
 * Such a valid callback set has all of its function pointers set except for
 * ah_tcp_conn_cbs::on_open and ah_tcp_conn_cbs::on_connect, which may or may
 * not be set.
 *
 * @param cbs Pointer to TCP connection callback set.
 * @return @c true only if @a cbs is @e valid. @c false otherwise.
 */
ah_extern bool ah_tcp_conn_cbs_is_valid_for_acceptance(const ah_tcp_conn_cbs_t* cbs);

/**
 * Checks if @a cbs is valid for handling connections initiated locally.
 *
 * Such a valid callback set has all of its function pointers set.
 *
 * @param cbs Pointer to TCP connection callback set.
 * @return @c true only if @a cbs is @e valid. @c false otherwise.
 */
ah_extern bool ah_tcp_conn_cbs_is_valid_for_connection(const ah_tcp_conn_cbs_t* cbs);

/** @} */

/**
 * @name TCP Input Buffers
 *
 * Operations on ah_tcp_in instances.
 *
 * @{
 */

/**
 * Allocates new input buffer, storing a pointer to it in @a owner_ptr.
 *
 * The allocated input buffer is stored to @a owner_ptr @e and contains its own
 * copy of @a owner_ptr. The buffer can later be detached from its owner by a
 * call to ah_tcp_in_detach(), which sets the copy to @c NULL and replaces the
 * pointer pointed to by @a owner_ptr with that of a new input buffer.
 *
 * Every input buffer allocated with this function must eventually be provided
 * to ah_tcp_in_free(). It is the responsibility of the owner of each instance
 * to make sure this is the case.
 *
 * @param owner_ptr Pointer to own pointer to allocated input buffer.
 * @return One of the following error codes: <ul>
 *   <li>@ref AH_ENONE     - The operation was successful.
 *   <li>@ref AH_EINVAL    - @a owner_ptr is @c NULL.
 *   <li>@ref AH_ENOMEM    - No enough heap memory available (ah_palloc() returned @c NULL).
 *   <li>@ref AH_EOVERFLOW - @c AH_PSIZE is too small for it to be possible to store both an
 *                           ah_tcp_in instance @e and have room for input data in a single page
 *                           provided by the page allocator (see ah_palloc()).
 * </ul>
 *
 * @note This function should primarily be of interest to those both wishing to
 *       implement their own TCP transports and need to intercept buffers (for
 *       the sake of decryption, for example).
 */
ah_extern ah_err_t ah_tcp_in_alloc_for(ah_tcp_in_t** owner_ptr);

/**
 * Detaches input buffer @a in from its owner.
 *
 * This function first allocates a new input buffer, disassociates @a in from
 * its current owner (most typically an ah_tcp_conn instance), and then
 * associates the newly allocated input buffer with that owner.
 *
 * @param in Pointer to input buffer.
 * @return One of the following error codes: <ul>
 *   <li>@ref AH_ENONE     - The operation was successful.
 *   <li>@ref AH_EINVAL    - @a in is @c NULL.
 *   <li>@ref AH_ENOMEM    - No enough heap memory available (ah_palloc() returned @c NULL).
 *   <li>@ref AH_EOVERFLOW - @c AH_PSIZE is too small for it to be possible to store both an
 *                           ah_tcp_in instance @e and have room for input data in a single page
 *                           provided by the page allocator (see ah_palloc()).
 *   <li>@ref AH_ESTATE    - @a in is currently not owned and cannot be detached.
 * </ul>
 *
 * @warning As the previous owner of @a in is no longer responsible for it or
 *          its memory, you must manually free it using ah_tcp_in_free() once
 *          you have no more use of it.
 */
ah_extern ah_err_t ah_tcp_in_detach(ah_tcp_in_t* in);

/**
 * Frees heap memory associated with @a in.
 *
 * @param in Pointer to input buffer.
 *
 * @warning Only free ah_tcp_in instances you own. Unless you explicitly call
 *          ah_tcp_in_alloc_for(), ah_tcp_in_detach() or in some other way is
 *          able to take ownership of your own instance, you are not going to
 *          need to call this function.
 *
 * @note This function does nothing if @a in is @c NULL.
 */
ah_extern void ah_tcp_in_free(ah_tcp_in_t* in);

/**
 * Moves the readable bytes of @a in to the beginning of its internal
 *        buffer.
 *
 * The internal buffer of @a in has a finite size. When data is written to that
 * internal buffer, a write pointer advances. If enough data is written to it,
 * the write pointer advances to the end of the buffer, making it impossible to
 * store further data to it. When data is read from the buffer, an internal
 * read pointer advances towards the write pointer. After a successful such
 * read operation, the memory between the beginning of the buffer and the read
 * pointer becomes inaccessible. This function moves the data between the read
 * and write pointers of the internal buffer to the beginning of that buffer.
 * This eliminates the inaccessible region and makes it possible to write more
 * data to the end of the buffer.
 *
 * TCP is a streaming transmission protocol. Whatever data is sent may arrive
 * split up into multiple segments at its intended receiver. Multiple such
 * segments may have to be awaited before a certain data object can be
 * interpreted correctly, and the last segment may contain the beginning of
 * another data object. In such a scenario, this function makes it possible to
 * move the received part of second data object to the beginning of the
 * internal buffer, preventing it from overflowing for data objects that are
 * smaller than the full size of that buffer.
 *
 * @param in Pointer to input buffer.
 * @return One of the following error codes: <ul>
 *   <li>@ref AH_ENONE     - The operation was successful.
 *   <li>@ref AH_EINVAL    - @a in is @c NULL.
 *   <li>@ref AH_EOVERFLOW - @a in is full. Nothing can be moved.
 * </ul>
 */
ah_extern ah_err_t ah_tcp_in_repackage(ah_tcp_in_t* in);

/**
 * Resets @a in, making all of its payload memory writable.
 *
 * @param in Pointer to input buffer to reset.
 *
 * @note This function does nothing if @a in is @c NULL.
 */
ah_extern void ah_tcp_in_reset(ah_tcp_in_t* in);

/** @} */

/**
 * @name TCP Listeners
 *
 * Operations on ah_tcp_listener instances. All such instances must be
 * initialized using ah_tcp_listener_init() before they are provided to any
 * other functions listed here. Any other requirements regarding the state of
 * listeners are described in the documentation of each respective function,
 * sometimes only via the error codes it lists.
 *
 * @{
 */

/**
 * Initializes @a ln for subsequent use.
 *
 * @param ln    Pointer to listener.
 * @param loop  Pointer to event loop.
 * @param trans Desired transport.
 * @param obs   Pointer to connection event observer.
 * @return One of the following error codes: <ul>
 *   <li>@ref AH_ENONE  - Operation successful.
 *   <li>@ref AH_EINVAL - @a ln is @c NULL, @a trans @c vtab is @c NULL or the @a trans
 *                        @c vtab->listener_init field is @c NULL.
 *   <li>Any additional code returned by the used TCP transport.
 * </ul>
 * The <em>default transport</em> may also cause any of the following error
 * codes to be returned: <ul>
 *   <li>@ref AH_EINVAL    - @a loop is @c NULL.
 *   <li>@ref AH_EINVAL    - @a trans @c ->vtab is invalid, as reported by
 *                           ah_tcp_trans_vtab_is_valid().
 *   <li>@ref AH_EINVAL    - @a obs @c ->cbs is invalid, as reported by
 *                           ah_tcp_listener_cbs_is_valid().
 *   <li>@ref AH_ENOMEM    - Heap memory could not be allocated for storing incoming connections.
 *   <li>@ref AH_EOVERFLOW - @c AH_PSIZE is too small for it to be possible to store both metadata
 *                           @e and have room for at least one incoming connection in a single page
 *                           provided by the page allocator (see ah_palloc()).
 * </ul>
 *
 * @note Every successfully initialized @a ln must eventually be provided to
 *       ah_tcp_listener_term().
 */
ah_extern ah_err_t ah_tcp_listener_init(ah_tcp_listener_t* ln, ah_loop_t* loop, ah_tcp_trans_t trans, ah_tcp_listener_obs_t obs);

/**
 * Schedules opening of @a ln, which must be initialized, and its binding to the
 * local network interface represented by @a laddr.
 *
 * If the return value of this function is @ref AH_ENONE, meaning that the open
 * attempt could indeed be scheduled, its result will eventually be presented
 * via the ah_tcp_listener_cbs::on_open callback of @a ln.
 *
 * @param ln    Pointer to listener.
 * @param laddr Pointer to socket address representing a local network interface
 *              through which the listener must later receive incoming
 *              connections. If opening is successful, the referenced address
 *              must remain valid for the remaining lifetime of @a ln. To bind
 *              to all or any local network interface, provide the wildcard
 *              address (see ah_sockaddr_ipv4_wildcard and
 *              ah_sockaddr_ipv6_wildcard). If you want the platform to chose
 *              port number automatically, specify port @c 0.
* @return One of the following error codes: <ul>
*   <li>@ref AH_ENONE         - Operation successful.
*   <li>@ref AH_EINVAL        - @a ln is @c NULL, @a trans @c vtab is @c NULL or the @a trans
*                               @c vtab->listener_open field is @c NULL.
*   <li>Any additional code returned by the used TCP transport.
* </ul>
* The <em>default transport</em> may also cause any of the following error
* codes to be returned: <ul>
 *   <li>@ref AH_EAFNOSUPPORT - @a laddr is not an IP-based address.
 *   <li>@ref AH_ECANCELED    - The event loop of @a ln is shutting down.
 *   <li>@ref AH_EINVAL       - @a ln or @a laddr is @c NULL.
 *   <li>@ref AH_ENOBUFS      - Not enough buffer space available.
 *   <li>@ref AH_ENOMEM       - Not enough heap memory available.
 *   <li>@ref AH_ESTATE       - @a ln is not closed.
 * </ul>
 *
 * @note Every successfully opened @a ln must eventually be provided to
 *       ah_tcp_listener_close() before it is provided to
 *       ah_tcp_listener_term().
 */
ah_extern ah_err_t ah_tcp_listener_open(ah_tcp_listener_t* ln, const ah_sockaddr_t* laddr);

/**
 * Schedules for @a ln, which must be open, to start listening for incoming
 * connections.
 *
 * If the return value of this function is @ref AH_ENONE, meaning that listening
 * could indeed be scheduled, its result will eventually be presented via the
 * ah_tcp_conn_cbs::on_listen callback of @a ln.
 *
 * @param ln       Pointer to listener.
 * @param backlog  Capacity, in connections, of the queue in which incoming
 *                 connections wait to get accepted. If @c 0, a platform
 *                 default will be chosen. If larger than some arbitrary
 *                 platform maximum, it will be set to that maximum.
 * @return One of the following error codes: <ul>
 *   <li>@ref AH_ENONE  - Operation successful.
 *   <li>@ref AH_EINVAL - @a ln is @c NULL, @a trans @c vtab is @c NULL or the @a trans
 *                        @c vtab->listener_listen field is @c NULL.
 *   <li>Any additional code returned by the used TCP transport.
 * </ul>
 * The <em>default transport</em> may also cause any of the following error
 * codes to be returned: <ul>
 *   <li>@ref AH_ECANCELED - The event loop of @a ln is shutting down.
 *   <li>@ref AH_EINVAL    - @a ln or @a conn_cbs is @c NULL.
 *   <li>@ref AH_EINVAL    - @c on_read, @c on_write or @c on_close of @a conn_cbs is @c NULL.
 *   <li>@ref AH_ENOBUFS   - Not enough buffer space available.
 *   <li>@ref AH_ENOMEM    - Not enough heap memory available.
 *   <li>@ref AH_ESTATE    - @a ln is not open.
 * </ul>
 *
 * @warning This function must be called with a successfully opened listener.
 *          An appropriate place to call this function is often going to be in
 *          an ah_tcp_listener_cbs::on_open callback after a check that opening
 *          was successful.
 */
ah_extern ah_err_t ah_tcp_listener_listen(ah_tcp_listener_t* ln, unsigned backlog);

/**
 * Schedules closing of @a ln.
 *
 * If the return value of this function is @ref AH_ENONE, meaning that the
 * closing could indeed be scheduled, its result will eventually be presented
 * via the ah_tcp_listener_cbs::on_close callback of @a ln.
 *
 * @param ln Pointer to listener.
 * @return One of the following error codes: <ul>
 *   <li>@ref AH_ENONE  - Operation successful.
 *   <li>@ref AH_EINVAL - @a ln is @c NULL, @a trans @c vtab is @c NULL or the @a trans
 *                        @c vtab->listener_close field is @c NULL.
 *   <li>Any additional code returned by the used TCP transport.
 * </ul>
 * The <em>default transport</em> may also cause the following error code to be
 * returned: <ul>
 *   <li>@ref AH_ESTATE - @a ln is already in a closing or closed state.
 * </ul>
 *
 * @note Any already accepted connections that are still open are unaffected by
 *       the listener being closed.
 */
ah_extern ah_err_t ah_tcp_listener_close(ah_tcp_listener_t* ln);

/**
 * Terminates @a ln, freeing any resources it holds.
 *
 * @param ln Pointer to listener.
 * @return One of the following error codes: <ul>
 *   <li>@ref AH_ENONE  - Operation successful.
 *   <li>@ref AH_EINVAL - @a ln is @c NULL, @a trans @c vtab is @c NULL or the @a trans
 *                        @c vtab->listener_term field is @c NULL.
 *   <li>Any additional code returned by the used TCP transport.
 * </ul>
 * The <em>default transport</em> may also cause the following error code to be
 * returned: <ul>
 *   <li>@ref AH_ESTATE - @a ln is not in a closed state.
 * </ul>
 *
 * @note Any already accepted connections that are still open are unaffected by
 *       the listener being terminated. It may, however, be the case that some
 *       resources @a ln shares with those connections are not freed until they
 *       are all closed.
 */
ah_extern ah_err_t ah_tcp_listener_term(ah_tcp_listener_t* ln);

/**
 * Checks the socket family of @a ln.
 *
 * For most TCP transports you may expect this socket family to be the same as
 * was specified in the call to ah_tcp_listener_open() through which @a ln was
 * first opened.
 *
 * @param ln Pointer to listener.
 * @return Socket family identifier or @c -1 if @a ln is @c NULL or some other
 *         error occurred. If the <em>default transport</em> is used, the
 *         following identifiers may be produced: <ul>
 *   <li>@ref AH_SOCKFAMILY_IPV4 - IPv4 family identifier.
 *   <li>@ref AH_SOCKFAMILY_IPV6 - IPv6 family identifier.
 * </ul>
 */
ah_extern int ah_tcp_listener_get_family(const ah_tcp_listener_t* ln);

/**
 * Stores local address bound by @a ln into @a laddr.
 *
 * If @a ln was opened with a zero port, this function @e should report what
 * concrete port was assigned to @a ln. If the <em>default transport</em>
 * is used and this call is not intercepted by an intermediary transport, this
 * function @e will report the assigned port.
 *
 * @param ln    Pointer to listener.
 * @param laddr Pointer to socket address to be set by this operation.
 * @return One of the following error codes: <ul>
 *   <li>@ref AH_ENONE                   - The operation was successful.
 *   <li>@ref AH_EINVAL                  - @a ln is @c NULL, @a ln @c ->vtab is @c NULL or @a ln
 *                                         @c ->vtab->listener_get_laddr is @c NULL.
 * </ul>
 * The <em>default transport</em> may also cause any of the following error
 * codes to be returned: <ul>
 *   <li>@ref AH_EINVAL                  - @a laddr is @c NULL.
 *   <li>@ref AH_ENETDOWN [Win32]        - The network subsystem has failed.
 *   <li>@ref AH_ENOBUFS [Darwin, Linux] - Not enough buffer space available.
 *   <li>@ref AH_ESTATE                  - @a ln is closed.
 * </ul>
 */
ah_extern ah_err_t ah_tcp_listener_get_laddr(const ah_tcp_listener_t* ln, ah_sockaddr_t* laddr);

/**
 * Gets pointer to event loop of @a ln.
 *
 * This event loop @e should be the same one as was provided when @a ln was
 * first initialized via a call to ah_tcp_listener_init(). If the <em>default
 * transport</em> is used and this function is not intercepted by an
 * intermediary transport, this function @e will return the mentioned original
 * event loop pointer.
 *
 * @param ln Pointer to listener.
 * @return Loop pointer, or @c NULL if @a ln is @c NULL, @a ln @c ->vtab
 *         is @c NULL or @a ln @c ->vtab->listener_get_loop is @c NULL. Also
 *         returns @c NULL if the loop pointer itself is equal to @c NULL.
 */
ah_extern ah_loop_t* ah_tcp_listener_get_loop(const ah_tcp_listener_t* ln);

/**
 * Gets the context pointer of the listener observer associated with @a conn.
 *
 * @param ln Pointer to listener.
 * @return Context pointer, or @c NULL if @a ln is @c NULL, @a ln @c ->vtab
 *         is @c NULL or @a ln @c ->vtab->listener_get_obs_ctx is @c NULL. Also
 *         returns @c NULL if the context pointer itself is equal to @c NULL.
 */
ah_extern void* ah_tcp_listener_get_obs_ctx(const ah_tcp_listener_t* ln);

/**
 * Checks if @a ln is in any closing or closed state.
 *
 * Also newly initialized and terminated listener are considered closed.
 *
 * @param ln Pointer to listener.
 * @return @c true only if @a ln is currently closing or closed. @c false if
 *         @a ln is @c NULL, @a ln @c ->vtab is @c NULL or @a ln
 *         @c ->vtab->listener_is_closed is @c NULL.
 */
ah_extern bool ah_tcp_listener_is_closed(ah_tcp_listener_t* ln);

/**
 * Sets the @e keep-alive option of @a ln to @a is_enabled.
 *
 * This option enables or disables keep-alive messaging for connections accepted
 * by @a ln. Generally, using such messaging means that @a ln automatically
 * sends messages at sensible times to check if the connection is in a usable
 * condition. The exact implications of this option depends on the platform.
 *
 * @param ln         Pointer to listener.
 * @param is_enabled Whether keep-alive is to be enabled or not.
 * @return One of the following error codes: <ul>
 *   <li>@ref AH_ENONE             - The operation was successful.
 *   <li>@ref AH_EINVAL            - @a ln is @c NULL, @a ln @c ->vtab is @c NULL or
 *                                   @a ln @c ->vtab->listener_set_keepalive is @c NULL.
 * </ul>
 * The <em>default transport</em> may also cause any of the following error
 * codes to be returned: <ul>
 *   <li>@ref AH_ENETDOWN [Win32] - The network subsystem has failed.
 *   <li>@ref AH_ENOBUFS [Darwin] - Not enough buffer space available.
 *   <li>@ref AH_ENOMEM [Darwin]  - Not enough heap memory available.
 *   <li>@ref AH_ESTATE           - @a ln is closed.
 * </ul>
 */
ah_extern ah_err_t ah_tcp_listener_set_keepalive(ah_tcp_listener_t* ln, bool is_enabled);

/**
 * Sets the @e no-delay option of @a ln to @a is_enabled.
 *
 * This option being enabled means that use of Nagle's algorithm is disabled for
 * accepted connections. The mentioned algorithm queues up messages for a short
 * time before sending them over the network. The purpose of this is to reduce
 * the number of TCP segments submitted over the used network. Its disadvantage
 * is that it may increase messaging latency.
 *
 * @param ln         Pointer to listener.
 * @param is_enabled Whether keep-alive is to be enabled or not.
 * @return One of the following error codes: <ul>
 *   <li>@ref AH_ENONE            - The operation was successful.
 *   <li>@ref AH_EINVAL           - @a ln is @c NULL, @a ln @c ->vtab is @c NULL or @a ln
 *                                  @c ->vtab->listener_set_nodelay is @c NULL.
 * </ul>
 * The <em>default transport</em> may also cause any of the following error
 * codes to be returned: <ul>
 *   <li>@ref AH_ENETDOWN [Win32] - The network subsystem has failed.
 *   <li>@ref AH_ENOBUFS [Darwin] - Not enough buffer space available.
 *   <li>@ref AH_ENOMEM [Darwin]  - Not enough heap memory available.
 *   <li>@ref AH_ESTATE           - @a ln is closed.
 * </ul>
 */
ah_extern ah_err_t ah_tcp_listener_set_nodelay(ah_tcp_listener_t* ln, bool is_enabled);

/**
 * Sets the <em>reuse address</em> option of @a ln to @a is_enabled.
 *
 * Address reuse generally means that a the specific combination of local
 * interface address and port number bound by this listener can be reused
 * right after it closes. Address reuse can lead to security implications as
 * it may enable a malicious process on the same platform to hijack a closed
 * connection.
 *
 * @param ln         Pointer to listener.
 * @param is_enabled Whether keep-alive is to be enabled or not.
 * @return One of the following error codes: <ul>
 *   <li>@ref AH_ENONE            - The operation was successful.
 *   <li>@ref AH_EINVAL           - @a ln is @c NULL, @a ln @c ->vtab is @c NULL or @a ln
 *                                  @c ->vtab->listener_set_reuseaddr is @c NULL.
 * </ul>
 * The <em>default transport</em> may also cause any of the following error
 * codes to be returned: <ul>
 *   <li>@ref AH_ENETDOWN [Win32] - The network subsystem has failed.
 *   <li>@ref AH_ENOBUFS [Darwin] - Not enough buffer space available.
 *   <li>@ref AH_ENOMEM [Darwin]  - Not enough heap memory available.
 *   <li>@ref AH_ESTATE           - @a ln is closed.
 * </ul>
 */
ah_extern ah_err_t ah_tcp_listener_set_reuseaddr(ah_tcp_listener_t* ln, bool is_enabled);

/** @} */

/**
 * @name TCP Listener Callback Sets
 *
 * Operations on ah_tcp_listener_cbs instances.
 *
 * @{
 */

/**
 * Checks if @a cbs is valid for handling TCP listener events.
 *
 * Such a valid callback set has all of its function pointers set.
 *
 * @param cbs Pointer to TCP listener callback set.
 * @return @c true only if @a cbs is @e valid. @c false otherwise.
 */
ah_extern bool ah_tcp_listener_cbs_is_valid(const ah_tcp_listener_cbs_t* cbs);

/** @} */

/**
 * @name TCP Output Buffers
 *
 * Operations on ah_tcp_out instances.
 *
 * @{
 */

/**
 * Dynamically allocates and partially initializes a TCP output buffer.
 *
 * Every output buffer allocated with this function must eventually be provided
 * to ah_tcp_out_free().
 *
 * Concretely, the page allocator (see ah_palloc()) is used to allocate the
 * returned buffer. All parts of the returned buffer are initialized, except
 * for the actual payload memory.
 *
 * @return Pointer to new output buffer, or @c NULL if the allocation failed.
 *
 * @warning If @c AH_PSIZE is configured to a too small value (see conf.h),
 *          this function always fails.
 */
ah_extern ah_tcp_out_t* ah_tcp_out_alloc(void);

/**
 * Frees output buffer previously allocated using ah_tcp_out_alloc().
 *
 * @param out Pointer to output buffer.
 *
 * @note If @a out is @c NULL, this function does nothing.
 */
ah_extern void ah_tcp_out_free(ah_tcp_out_t* out);

/** @} */

/**
 * @name TCP Transports
 *
 * Operations on ah_tcp_trans and related type instances.
 *
 * @{
 */

/**
 * Gets a copy of the root TCP transport.
 *
 * The root TCP transport directly utilizes the network subsystem of the
 * current platform. This transport may be used directly with
 * ah_tcp_conn_init() and ah_tcp_listener_init() to establish plain TCP
 * connections, which is to say that they are not encrypted or analyzed in any
 * way.
 *
 * @return Copy of default TCP transport.
 */
ah_extern ah_tcp_trans_t ah_tcp_trans_get_default(void);

/**
 * Checks if every field of @a vtab is set, making it valid for use as part of a
 * TCP transport.
 *
 * @param vtab Pointer to virtual function table.
 * @return @c true only if @a vtab is not @c NULL and is valid. @c false
 *         otherwise.
 */
ah_extern bool ah_tcp_trans_vtab_is_valid(const ah_tcp_trans_vtab_t* vtab);

/** @} */

#endif
