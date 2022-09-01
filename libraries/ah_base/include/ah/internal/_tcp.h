// SPDX-License-Identifier: EPL-2.0

#ifndef AH_INTERNAL_TCP_H_
#define AH_INTERNAL_TCP_H_

#include "../defs.h"
#include "collections/slab.h"

#if AH_USE_IOCP
# include "_tcp-iocp.h"
#elif AH_USE_KQUEUE
# include "_tcp-kqueue.h"
#elif AH_USE_URING
# include "_tcp-uring.h"
#endif

#define AH_I_TCP_CONN_STATE_TERMINATED  0u
#define AH_I_TCP_CONN_STATE_INITIALIZED 1u
#define AH_I_TCP_CONN_STATE_CLOSING     2u
#define AH_I_TCP_CONN_STATE_CLOSED      3u
#define AH_I_TCP_CONN_STATE_OPEN        4u
#define AH_I_TCP_CONN_STATE_CONNECTING  5u
#define AH_I_TCP_CONN_STATE_CONNECTED   6u // Writes allowed.
#define AH_I_TCP_CONN_STATE_READING     7u // Writes allowed.

#define AH_I_TCP_LISTENER_STATE_TERMINATED  0u
#define AH_I_TCP_LISTENER_STATE_INITIALIZED 1u
#define AH_I_TCP_LISTENER_STATE_CLOSING     2u
#define AH_I_TCP_LISTENER_STATE_CLOSED      3u
#define AH_I_TCP_LISTENER_STATE_OPEN        4u
#define AH_I_TCP_LISTENER_STATE_LISTENING   5u

#define AH_I_TCP_CONN_FIELDS     \
 ah_loop_t* _loop;               \
                                 \
 ah_tcp_trans_t _trans;          \
 ah_tcp_conn_obs_t _obs;         \
                                 \
 struct ah_i_slab* _owning_slab; \
                                 \
 ah_tcp_in_t* _in;               \
                                 \
 bool _is_ipv6;                  \
 uint8_t _shutdown_flags;        \
 uint8_t _state;                 \
                                 \
 AH_I_TCP_CONN_PLATFORM_FIELDS

#define AH_I_TCP_IN_FIELDS \
 ah_tcp_in_t** _owner_ptr;

#define AH_I_TCP_LISTENER_FIELDS \
 ah_loop_t* _loop;               \
                                 \
 ah_tcp_trans_t _trans;          \
 ah_tcp_listener_obs_t _obs;     \
                                 \
 struct ah_i_slab _conn_slab;    \
                                 \
 bool _is_ipv6;                  \
 uint8_t _state;                 \
                                 \
 AH_I_TCP_LISTENER_PLATFORM_FIELDS

#define AH_I_TCP_OUT_FIELDS \
 void* _owner;              \
                            \
 AH_I_TCP_OUT_PLATFORM_FIELDS

#endif
