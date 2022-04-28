// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0.
//
// SPDX-License-Identifier: EPL-2.0

#ifndef AH_INTERNAL_URING_TCP_H_
#define AH_INTERNAL_URING_TCP_H_

#define AH_I_TCP_LISTEN_CTX_PLATFORM_FIELDS                                                                            \
    ah_sockaddr_t _remote_addr;                                                                                        \
    socklen_t _remote_addr_len;

#define AH_I_TCP_READ_CTX_PLATFORM_FIELDS ah_bufs_t _bufs;
#define AH_I_TCP_SOCK_PLATFORM_FIELDS     ah_i_sockfd_t _fd;
#define AH_I_TCP_WRITE_CTX_PLATFORM_FIELDS

#endif
