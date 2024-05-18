// SPDX-License-Identifier: EPL-2.0

#include <ah/unit.h>

AH_UNIT_SUITE(err)
{
    AH_UNIT_TEST("ah_err_get_s() returns name of each code in err.h")
    {
        struct ah_Case {
            ah_err_t input;
            const char* output;
        };

        static const struct ah_Case cases[] = {
            { AH_OK, "OK" },
            { AH_E2BIG, "E2BIG" },
            { AH_EACCES, "EACCES" },
            { AH_EADDRINUSE, "EADDRINUSE" },
            { AH_EADDRNOTAVAIL, "EADDRNOTAVAIL" },
            { AH_EAFNOSUPPORT, "EAFNOSUPPORT" },
            { AH_EAGAIN, "EAGAIN" },
            { AH_EALREADY, "EALREADY" },
            { AH_EBADF, "EBADF" },
            { AH_EBADMSG, "EBADMSG" },
            { AH_EBUSY, "EBUSY" },
            { AH_ECANCELED, "ECANCELED" },
            { AH_ECHILD, "ECHILD" },
            { AH_ECLOCKRANGE, "ECLOCKRANGE" },
            { AH_ECLOCKUNSET, "ECLOCKUNSET" },
            { AH_ECONNABORTED, "ECONNABORTED" },
            { AH_ECONNREFUSED, "ECONNREFUSED" },
            { AH_ECONNRESET, "ECONNRESET" },
            { AH_EDEADLK, "EDEADLK" },
            { AH_EDEP, "EDEP" },
            { AH_EDESTADDRREQ, "EDESTADDRREQ" },
            { AH_EDOM, "EDOM" },
            { AH_EDQUOT, "EDQUOT" },
            { AH_EEOF, "EEOF" },
            { AH_EEXIST, "EEXIST" },
            { AH_EFAULT, "EFAULT" },
            { AH_EFBIG, "EFBIG" },
            { AH_EHOSTDOWN, "EHOSTDOWN" },
            { AH_EHOSTUNREACH, "EHOSTUNREACH" },
            { AH_EIDRM, "EIDRM" },
            { AH_EILSEQ, "EILSEQ" },
            { AH_EINPROGRESS, "EINPROGRESS" },
            { AH_EINTERN, "EINTERN" },
            { AH_EINTR, "EINTR" },
            { AH_EINVAL, "EINVAL" },
            { AH_EIO, "EIO" },
            { AH_EISCONN, "EISCONN" },
            { AH_EISDIR, "EISDIR" },
            { AH_ELOOP, "ELOOP" },
            { AH_EMFILE, "EMFILE" },
            { AH_EMLINK, "EMLINK" },
            { AH_EMSGSIZE, "EMSGSIZE" },
            { AH_EMULTIHOP, "EMULTIHOP" },
            { AH_ENAMETOOLONG, "ENAMETOOLONG" },
            { AH_ENETDOWN, "ENETDOWN" },
            { AH_ENETRESET, "ENETRESET" },
            { AH_ENETUNREACH, "ENETUNREACH" },
            { AH_ENFILE, "ENFILE" },
            { AH_ENOBUFS, "ENOBUFS" },
            { AH_ENODATA, "ENODATA" },
            { AH_ENODEV, "ENODEV" },
            { AH_ENOENT, "ENOENT" },
            { AH_ENOEXEC, "ENOEXEC" },
            { AH_ENOLCK, "ENOLCK" },
            { AH_ENOLINK, "ENOLINK" },
            { AH_ENOMEM, "ENOMEM" },
            { AH_ENOMSG, "ENOMSG" },
            { AH_ENOPROTOOPT, "ENOPROTOOPT" },
            { AH_ENOSPC, "ENOSPC" },
            { AH_ENOSR, "ENOSR" },
            { AH_ENOSTR, "ENOSTR" },
            { AH_ENOSYS, "ENOSYS" },
            { AH_ENOTBLK, "ENOTBLK" },
            { AH_ENOTCONN, "ENOTCONN" },
            { AH_ENOTDIR, "ENOTDIR" },
            { AH_ENOTEMPTY, "ENOTEMPTY" },
            { AH_ENOTRECOVERABLE, "ENOTRECOVERABLE" },
            { AH_ENOTSOCK, "ENOTSOCK" },
            { AH_ENXIO, "ENXIO" },
            { AH_EOPNOTSUPP, "EOPNOTSUPP" },
            { AH_EOVERFLOW, "EOVERFLOW" },
            { AH_EOWNERDEAD, "EOWNERDEAD" },
            { AH_EPERM, "EPERM" },
            { AH_EPFNOSUPPORT, "EPFNOSUPPORT" },
            { AH_EPIPE, "EPIPE" },
            { AH_EPROTO, "EPROTO" },
            { AH_EPROTONOSUPPORT, "EPROTONOSUPPORT" },
            { AH_EPROTOTYPE, "EPROTOTYPE" },
            { AH_ERANGE, "ERANGE" },
            { AH_EROFS, "EROFS" },
            { AH_ESHUTDOWN, "ESHUTDOWN" },
            { AH_ESOCKTNOSUPPORT, "ESOCKTNOSUPPORT" },
            { AH_ESPIPE, "ESPIPE" },
            { AH_ESRCH, "ESRCH" },
            { AH_ESTALE, "ESTALE" },
            { AH_ESTATE, "ESTATE" },
            { AH_ESYNTAX, "ESYNTAX" },
            { AH_ETIME, "ETIME" },
            { AH_ETIMEDOUT, "ETIMEDOUT" },
            { AH_ETOOMANYREFS, "ETOOMANYREFS" },
            { AH_ETXTBSY, "ETXTBSY" },
            { AH_EUSERS, "EUSERS" },
            { AH_EXDEV, "EXDEV" },
        };

        for (size_t i = 0u; i < sizeof(cases) / sizeof(struct ah_Case); i++) {
            struct ah_Case c = cases[i];

            AH_UNIT_CASE("ERR[%d]", (int) c.input)
            {
                AH_UNIT_EQ_STR(c.output, ah_err_get_s(c.input));
            }
        }
    }
}
