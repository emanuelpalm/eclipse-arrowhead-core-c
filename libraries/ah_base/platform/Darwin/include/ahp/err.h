// SPDX-License-Identifier: EPL-2.0

#ifndef AHP_ERR_H
#define AHP_ERR_H

#include <errno.h>

#define AHP_OK 0

#define AHP_E2BIG           E2BIG
#define AHP_EACCES          EACCES
#define AHP_EADDRINUSE      EADDRINUSE
#define AHP_EADDRNOTAVAIL   EADDRNOTAVAIL
#define AHP_EAFNOSUPPORT    EAFNOSUPPORT
#define AHP_EAGAIN          EAGAIN
#define AHP_EALREADY        EALREADY
#define AHP_EBADF           EBADF
#define AHP_EBADMSG         EBADMSG
#define AHP_EBUSY           EBUSY
#define AHP_ECANCELED       ECANCELED
#define AHP_ECHILD          ECHILD
#define AHP_ECONNABORTED    ECONNABORTED
#define AHP_ECONNREFUSED    ECONNREFUSED
#define AHP_ECONNRESET      ECONNRESET
#define AHP_EDEADLK         EDEADLK
#define AHP_EDEP            (ELAST + 1)
#define AHP_EDESTADDRREQ    EDESTADDRREQ
#define AHP_EDOM            EDOM
#define AHP_EDQUOT          EDQUOT
#define AHP_EEOF            (ELAST + 2)
#define AHP_EEXIST          EEXIST
#define AHP_EFAULT          EFAULT
#define AHP_EFBIG           EFBIG
#define AHP_EHOSTDOWN       EHOSTDOWN
#define AHP_EHOSTUNREACH    EHOSTUNREACH
#define AHP_EIDRM           EIDRM
#define AHP_EILSEQ          EILSEQ
#define AHP_EINPROGRESS     EINPROGRESS
#define AHP_EINTERN         (ELAST + 3)
#define AHP_EINTR           EINTR
#define AHP_EINVAL          EINVAL
#define AHP_EIO             EIO
#define AHP_EISCONN         EISCONN
#define AHP_EISDIR          EISDIR
#define AHP_ELOOP           ELOOP
#define AHP_EMFILE          EMFILE
#define AHP_EMLINK          EMLINK
#define AHP_EMSGSIZE        EMSGSIZE
#define AHP_EMULTIHOP       EMULTIHOP
#define AHP_ENAMETOOLONG    ENAMETOOLONG
#define AHP_ENETDOWN        ENETDOWN
#define AHP_ENETRESET       ENETRESET
#define AHP_ENETUNREACH     ENETUNREACH
#define AHP_ENFILE          ENFILE
#define AHP_ENOBUFS         ENOBUFS
#define AHP_ENODATA         ENODATA
#define AHP_ENODEV          ENODEV
#define AHP_ENOENT          ENOENT
#define AHP_ENOEXEC         ENOEXEC
#define AHP_ENOLCK          ENOLCK
#define AHP_ENOLINK         ENOLINK
#define AHP_ENOMEM          ENOMEM
#define AHP_ENOMSG          ENOMSG
#define AHP_ENOPROTOOPT     ENOPROTOOPT
#define AHP_ENOSPC          ENOSPC
#define AHP_ENOSR           ENOSR
#define AHP_ENOSTR          ENOSTR
#define AHP_ENOSYS          ENOSYS
#define AHP_ENOTBLK         ENOTBLK
#define AHP_ENOTCONN        ENOTCONN
#define AHP_ENOTDIR         ENOTDIR
#define AHP_ENOTEMPTY       ENOTEMPTY
#define AHP_ENOTRECOVERABLE ENOTRECOVERABLE
#define AHP_ENOTSOCK        ENOTSOCK
#define AHP_ENXIO           ENXIO
#define AHP_EOPNOTSUPP      EOPNOTSUPP
#define AHP_EOVERFLOW       EOVERFLOW
#define AHP_EOWNERDEAD      EOWNERDEAD
#define AHP_EPERM           EPERM
#define AHP_EPFNOSUPPORT    EPFNOSUPPORT
#define AHP_EPIPE           EPIPE
#define AHP_EPROTO          EPROTO
#define AHP_EPROTONOSUPPORT EPROTONOSUPPORT
#define AHP_EPROTOTYPE      EPROTOTYPE
#define AHP_ERANGE          ERANGE
#define AHP_EROFS           EROFS
#define AHP_ESHUTDOWN       ESHUTDOWN
#define AHP_ESOCKTNOSUPPORT ESOCKTNOSUPPORT
#define AHP_ESPIPE          ESPIPE
#define AHP_ESRCH           ESRCH
#define AHP_ESTALE          ESTALE
#define AHP_ESTATE          (ELAST + 4)
#define AHP_ESYNTAX         (ELAST + 5)
#define AHP_ETIME           ETIME
#define AHP_ETIMEDOUT       ETIMEDOUT
#define AHP_ETOOMANYREFS    ETOOMANYREFS
#define AHP_ETXTBSY         ETXTBSY
#define AHP_EUSERS          EUSERS
#define AHP_EXDEV           EXDEV

const char* ahp_err_get_s(int err);

#endif
