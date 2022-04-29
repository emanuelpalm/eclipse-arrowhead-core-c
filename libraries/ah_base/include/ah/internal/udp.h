// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0.
//
// SPDX-License-Identifier: EPL-2.0

#ifndef AH_INTERNAL_UDP_H_
#define AH_INTERNAL_UDP_H_

#include "../defs.h"

#if AH_USE_IOCP
#    include "iocp/udp.h"
#elif AH_USE_KQUEUE
#    include "kqueue/udp.h"
#elif AH_USE_URING
#    include "uring/udp.h"
#endif

#define AH_I_UDP_SOCK_FIELDS                                                                                           \
    ah_loop_t* _loop;                                                                                                  \
    const ah_udp_sock_vtab_t* _vtab;                                                                                   \
    void* _trans_data;                                                                                                 \
    void* _user_data;                                                                                                  \
    bool _is_ipv6;                                                                                                     \
    bool _is_open;                                                                                                     \
    bool _is_receiving;                                                                                                \
    bool _is_sending;                                                                                                  \
    AH_I_UDP_SOCK_PLATFORM_FIELDS

#define AH_I_UDP_TRANS_FIELDS                                                                                          \
    ah_loop_t* _loop;                                                                                                  \
    const ah_udp_trans_vtab_t* _vtab;                                                                                  \
    void* _trans_data;

#endif
