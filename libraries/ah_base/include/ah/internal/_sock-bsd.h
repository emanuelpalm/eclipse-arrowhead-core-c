// SPDX-License-Identifier: EPL-2.0

// SPDX-License-Identifier: EPL-2.0

#ifndef AH_INTERNAL_POSIX_SOCK_H_
#define AH_INTERNAL_POSIX_SOCK_H_

#include "../defs.h"

#if AH_HAS_POSIX
# include <netinet/in.h>
#elif AH_IS_WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>

# include <winsock2.h>
#endif

#ifdef SIN6_LEN
# define AH_I_SOCKADDR_HAS_SIZE 1
#endif

#define AH_I_SOCKFAMILY_IPV4 AF_INET
#define AH_I_SOCKFAMILY_IPV6 AF_INET6

#if AH_HAS_POSIX
typedef int ah_i_sockfd_t;
typedef socklen_t ah_i_socklen_t;
#elif AH_IS_WIN32
typedef SOCKET ah_i_sockfd_t;
typedef int ah_i_socklen_t;
#endif

ah_extern ah_i_socklen_t ah_i_sockaddr_get_size(const ah_sockaddr_t* sockaddr);

static inline ah_sockaddr_t* ah_i_sockaddr_from_bsd(struct sockaddr* sockaddr)
{
    return (ah_sockaddr_t*) sockaddr;
}

static inline const ah_sockaddr_t* ah_i_sockaddr_const_from_bsd(const struct sockaddr* sockaddr)
{
    return (const ah_sockaddr_t*) sockaddr;
}

static inline struct sockaddr* ah_i_sockaddr_into_bsd(ah_sockaddr_t* sockaddr)
{
    return (struct sockaddr*) sockaddr;
}

static inline const struct sockaddr* ah_i_sockaddr_const_into_bsd(const ah_sockaddr_t* sockaddr)
{
    return (const struct sockaddr*) sockaddr;
}

ah_extern ah_err_t ah_i_sock_open(ah_loop_t* loop, int sockfamily, int type, ah_i_sockfd_t* fd);
ah_extern ah_err_t ah_i_sock_open_bind(ah_loop_t* loop, const ah_sockaddr_t* laddr, int type, ah_i_sockfd_t* fd);
ah_extern ah_err_t ah_i_sock_close(ah_i_sockfd_t fd);

ah_extern ah_err_t ah_i_sock_getsockname(ah_i_sockfd_t fd, ah_sockaddr_t* laddr);
ah_extern ah_err_t ah_i_sock_getpeername(ah_i_sockfd_t fd, ah_sockaddr_t* raddr);

ah_extern ah_err_t ah_i_sock_setsockopt(ah_i_sockfd_t fd, int level, int name, const void* value, ah_i_socklen_t size);

ah_extern ah_err_t ah_i_sock_shutdown(ah_i_sockfd_t fd, int flags);

#endif
