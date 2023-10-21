// SPDX-License-Identifier: EPL-2.0

#include "ahp/err.h"

#include "ahp/def.h"

#include <stdio.h>
#include <string.h>

const char* ahp_err_get_s(int err)
{
    static ahp_thread_local char buf[64u];

    int res;

    switch (err) {
    case AHP_OK:
        return "OK";

        // Darwin codes.

#ifdef EPERM
    case EPERM:
        return "EPERM";
#endif

#ifdef ENOENT
    case ENOENT:
        return "ENOENT";
#endif

#ifdef ESRCH
    case ESRCH:
        return "ESRCH";
#endif

#ifdef EINTR
    case EINTR:
        return "EINTR";
#endif

#ifdef EIO
    case EIO:
        return "EIO";
#endif

#ifdef ENXIO
    case ENXIO:
        return "ENXIO";
#endif

#ifdef E2BIG
    case E2BIG:
        return "E2BIG";
#endif

#ifdef ENOEXEC
    case ENOEXEC:
        return "ENOEXEC";
#endif

#ifdef EBADF
    case EBADF:
        return "EBADF";
#endif

#ifdef ECHILD
    case ECHILD:
        return "ECHILD";
#endif

#if defined(EDEADLK) && (!defined(EAGAIN) || EDEADLK != EAGAIN)
    case EDEADLK:
        return "EDEADLK";
#endif

#ifdef ENOMEM
    case ENOMEM:
        return "ENOMEM";
#endif

#ifdef EACCES
    case EACCES:
        return "EACCES";
#endif

#ifdef EFAULT
    case EFAULT:
        return "EFAULT";
#endif

#ifdef ENOTBLK
    case ENOTBLK:
        return "ENOTBLK";
#endif

#ifdef EBUSY
    case EBUSY:
        return "EBUSY";
#endif

#ifdef EEXIST
    case EEXIST:
        return "EEXIST";
#endif

#ifdef EXDEV
    case EXDEV:
        return "EXDEV";
#endif

#ifdef ENODEV
    case ENODEV:
        return "ENODEV";
#endif

#ifdef ENOTDIR
    case ENOTDIR:
        return "ENOTDIR";
#endif

#ifdef EISDIR
    case EISDIR:
        return "EISDIR";
#endif

#ifdef EINVAL
    case EINVAL:
        return "EINVAL";
#endif

#ifdef ENFILE
    case ENFILE:
        return "ENFILE";
#endif

#ifdef EMFILE
    case EMFILE:
        return "EMFILE";
#endif

#ifdef ENOTTY
    case ENOTTY:
        return "ENOTTY";
#endif

#ifdef ETXTBSY
    case ETXTBSY:
        return "ETXTBSY";
#endif

#ifdef EFBIG
    case EFBIG:
        return "EFBIG";
#endif

#ifdef ENOSPC
    case ENOSPC:
        return "ENOSPC";
#endif

#ifdef ESPIPE
    case ESPIPE:
        return "ESPIPE";
#endif

#ifdef EROFS
    case EROFS:
        return "EROFS";
#endif

#ifdef EMLINK
    case EMLINK:
        return "EMLINK";
#endif

#ifdef EPIPE
    case EPIPE:
        return "EPIPE";
#endif

#ifdef EDOM
    case EDOM:
        return "EDOM";
#endif

#ifdef ERANGE
    case ERANGE:
        return "ERANGE";
#endif

#ifdef EAGAIN
    case EAGAIN:
        return "EAGAIN";
#endif

#if defined(EWOULDBLOCK) && (!defined(EAGAIN) || EWOULDBLOCK != EAGAIN)
    case EWOULDBLOCK:
        return "EWOULDBLOCK";
#endif

#ifdef EINPROGRESS
    case EINPROGRESS:
        return "EINPROGRESS";
#endif

#ifdef EALREADY
    case EALREADY:
        return "EALREADY";
#endif

#ifdef ENOTSOCK
    case ENOTSOCK:
        return "ENOTSOCK";
#endif

#ifdef EDESTADDRREQ
    case EDESTADDRREQ:
        return "EDESTADDRREQ";
#endif

#ifdef EMSGSIZE
    case EMSGSIZE:
        return "EMSGSIZE";
#endif

#ifdef EPROTOTYPE
    case EPROTOTYPE:
        return "EPROTOTYPE";
#endif

#ifdef ENOPROTOOPT
    case ENOPROTOOPT:
        return "ENOPROTOOPT";
#endif

#ifdef EPROTONOSUPPORT
    case EPROTONOSUPPORT:
        return "EPROTONOSUPPORT";
#endif

#ifdef ESOCKTNOSUPPORT
    case ESOCKTNOSUPPORT:
        return "ESOCKTNOSUPPORT";
#endif

#ifdef ENOTSUP
    case ENOTSUP:
        return "ENOTSUP";
#endif

#if defined(EOPNOTSUPP) && (!defined(ENOTSUP) || EOPNOTSUPP != ENOTSUP)
    case EOPNOTSUPP:
        return "EOPNOTSUPP";
#endif

#ifdef EPFNOSUPPORT
    case EPFNOSUPPORT:
        return "EPFNOSUPPORT";
#endif

#ifdef EAFNOSUPPORT
    case EAFNOSUPPORT:
        return "EAFNOSUPPORT";
#endif

#ifdef EADDRINUSE
    case EADDRINUSE:
        return "EADDRINUSE";
#endif

#ifdef EADDRNOTAVAIL
    case EADDRNOTAVAIL:
        return "EADDRNOTAVAIL";
#endif

#ifdef ENETDOWN
    case ENETDOWN:
        return "ENETDOWN";
#endif

#ifdef ENETUNREACH
    case ENETUNREACH:
        return "ENETUNREACH";
#endif

#ifdef ENETRESET
    case ENETRESET:
        return "ENETRESET";
#endif

#ifdef ECONNABORTED
    case ECONNABORTED:
        return "ECONNABORTED";
#endif

#ifdef ECONNRESET
    case ECONNRESET:
        return "ECONNRESET";
#endif

#ifdef ENOBUFS
    case ENOBUFS:
        return "ENOBUFS";
#endif

#ifdef EISCONN
    case EISCONN:
        return "EISCONN";
#endif

#ifdef ENOTCONN
    case ENOTCONN:
        return "ENOTCONN";
#endif

#ifdef ESHUTDOWN
    case ESHUTDOWN:
        return "ESHUTDOWN";
#endif

#ifdef ETOOMANYREFS
    case ETOOMANYREFS:
        return "ETOOMANYREFS";
#endif

#ifdef ETIMEDOUT
    case ETIMEDOUT:
        return "ETIMEDOUT";
#endif

#ifdef ECONNREFUSED
    case ECONNREFUSED:
        return "ECONNREFUSED";
#endif

#ifdef ELOOP
    case ELOOP:
        return "ELOOP";
#endif

#ifdef ENAMETOOLONG
    case ENAMETOOLONG:
        return "ENAMETOOLONG";
#endif

#ifdef EHOSTDOWN
    case EHOSTDOWN:
        return "EHOSTDOWN";
#endif

#ifdef EHOSTUNREACH
    case EHOSTUNREACH:
        return "EHOSTUNREACH";
#endif

#ifdef ENOTEMPTY
    case ENOTEMPTY:
        return "ENOTEMPTY";
#endif

#ifdef EPROCLIM
    case EPROCLIM:
        return "EPROCLIM";
#endif

#ifdef EUSERS
    case EUSERS:
        return "EUSERS";
#endif

#ifdef EDQUOT
    case EDQUOT:
        return "EDQUOT";
#endif

#ifdef ESTALE
    case ESTALE:
        return "ESTALE";
#endif

#ifdef EREMOTE
    case EREMOTE:
        return "EREMOTE";
#endif

#ifdef EBADRPC
    case EBADRPC:
        return "EBADRPC";
#endif

#ifdef ERPCMISMATCH
    case ERPCMISMATCH:
        return "ERPCMISMATCH";
#endif

#ifdef EPROGUNAVAIL
    case EPROGUNAVAIL:
        return "EPROGUNAVAIL";
#endif

#ifdef EPROGMISMATCH
    case EPROGMISMATCH:
        return "EPROGMISMATCH";
#endif

#ifdef EPROCUNAVAIL
    case EPROCUNAVAIL:
        return "EPROCUNAVAIL";
#endif

#ifdef ENOLCK
    case ENOLCK:
        return "ENOLCK";
#endif

#ifdef ENOSYS
    case ENOSYS:
        return "ENOSYS";
#endif

#ifdef EFTYPE
    case EFTYPE:
        return "EFTYPE";
#endif

#ifdef EAUTH
    case EAUTH:
        return "EAUTH";
#endif

#ifdef ENEEDAUTH
    case ENEEDAUTH:
        return "ENEEDAUTH";
#endif

#ifdef EPWROFF
    case EPWROFF:
        return "EPWROFF";
#endif

#ifdef EDEVERR
    case EDEVERR:
        return "EDEVERR";
#endif

#ifdef EOVERFLOW
    case EOVERFLOW:
        return "EOVERFLOW";
#endif

#ifdef EBADEXEC
    case EBADEXEC:
        return "EBADEXEC";
#endif

#ifdef EBADARCH
    case EBADARCH:
        return "EBADARCH";
#endif

#ifdef ESHLIBVERS
    case ESHLIBVERS:
        return "ESHLIBVERS";
#endif

#ifdef EBADMACHO
    case EBADMACHO:
        return "EBADMACHO";
#endif

#ifdef ECANCELED
    case ECANCELED:
        return "ECANCELED";
#endif

#ifdef EIDRM
    case EIDRM:
        return "EIDRM";
#endif

#ifdef ENOMSG
    case ENOMSG:
        return "ENOMSG";
#endif

#ifdef EILSEQ
    case EILSEQ:
        return "EILSEQ";
#endif

#ifdef ENOATTR
    case ENOATTR:
        return "ENOATTR";
#endif

#ifdef EBADMSG
    case EBADMSG:
        return "EBADMSG";
#endif

#ifdef EMULTIHOP
    case EMULTIHOP:
        return "EMULTIHOP";
#endif

#ifdef ENODATA
    case ENODATA:
        return "ENODATA";
#endif

#ifdef ENOLINK
    case ENOLINK:
        return "ENOLINK";
#endif

#ifdef ENOSR
    case ENOSR:
        return "ENOSR";
#endif

#ifdef ENOSTR
    case ENOSTR:
        return "ENOSTR";
#endif

#ifdef EPROTO
    case EPROTO:
        return "EPROTO";
#endif

#ifdef ETIME
    case ETIME:
        return "ETIME";
#endif

#ifdef ENOPOLICY
    case ENOPOLICY:
        return "ENOPOLICY";
#endif

#ifdef ENOTRECOVERABLE
    case ENOTRECOVERABLE:
        return "ENOTRECOVERABLE";
#endif

#ifdef EOWNERDEAD
    case EOWNERDEAD:
        return "EOWNERDEAD";
#endif

#ifdef EQFULL
    case EQFULL:
        return "EQFULL";
#endif

        // Custom codes.

    case AHP_EDEP:
        return "EDEP";

    case AHP_EEOF:
        return "EEOF";

    case AHP_EINTERN:
        return "EINTERN";

    case AHP_ESTATE:
        return "ESTATE";

    case AHP_ESYNTAX:
        return "ESYNTAX";

    default:
        res = snprintf(buf, sizeof(buf), "ERR[%d]: ", err);
        if (res < 0) {
            return strncpy(buf, "ERR[?]", sizeof(buf));
        }
        if (((size_t) res) >= sizeof(buf)) {
            return buf;
        }

        (void) strerror_r(err, buf, sizeof(buf) - res);

        return buf;
    }
}
