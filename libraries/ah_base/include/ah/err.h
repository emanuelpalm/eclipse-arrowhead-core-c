// SPDX-License-Identifier: EPL-2.0

#ifndef AH_ERR_H
#define AH_ERR_H

/**
 * @file
 * Error codes.
 *
 * Error codes that can be represented by the ah_err_t type as well as
 * functions for inspecting such codes.
 */

#include "def.h"

#include <ahp/err.h>

/**
 * Error code signifying the absence of an error.
 *
 * While this will be defined as @c 0 on virtually all platforms, this is not
 * guaranteed. Portable applications must explicitly use this constant when
 * checking if an operation succeeded.
 */
#define AH_OK AHP_OK

/**
 * @name Error Codes
 * @defgroup ah_err_codes
 *
 * These error codes are defined on every platform supported by the base
 * library. Excluding a few custom codes, most are derived from the POSIX
 * specification. Additional error codes may be produced by base library
 * functions with platform-specific implementations.
 *
 * @{
 */
#define AH_E2BIG           AHP_E2BIG           ///< Argument list too long.
#define AH_EACCES          AHP_EACCES          ///< Permission denied.
#define AH_EADDRINUSE      AHP_EADDRINUSE      ///< Address in use.
#define AH_EADDRNOTAVAIL   AHP_EADDRNOTAVAIL   ///< Address not available.
#define AH_EAFNOSUPPORT    AHP_EAFNOSUPPORT    ///< Address family not supported.
#define AH_EAGAIN          AHP_EAGAIN          ///< Try again.
#define AH_EALREADY        AHP_EALREADY        ///< Already in progress.
#define AH_EBADF           AHP_EBADF           ///< Bad file descriptor.
#define AH_EBADMSG         AHP_EBADMSG         ///< Bad message.
#define AH_EBUSY           AHP_EBUSY           ///< Device or resource busy.
#define AH_ECANCELED       AHP_ECANCELED       ///< Operation canceled.
#define AH_ECHILD          AHP_ECHILD          ///< No child processes.
#define AH_ECLOCKRANGE     AHP_ECLOCKRANGE     ///< Clock time outside representable range.
#define AH_ECLOCKUNSET     AHP_ECLOCKUNSET     ///< Clock time never set.
#define AH_ECONNABORTED    AHP_ECONNABORTED    ///< Connection aborted.
#define AH_ECONNREFUSED    AHP_ECONNREFUSED    ///< Connection refused.
#define AH_ECONNRESET      AHP_ECONNRESET      ///< Connection reset.
#define AH_EDEADLK         AHP_EDEADLK         ///< Deadlock would occur.
#define AH_EDEP            AHP_EDEP            ///< Consult dependency for error details.
#define AH_EDESTADDRREQ    AHP_EDESTADDRREQ    ///< Destination address required.
#define AH_EDOM            AHP_EDOM            ///< Arithmetic argument outside accepted domain.
#define AH_EDQUOT          AHP_EDQUOT          ///< Disc quota exceeded.
#define AH_EEOF            AHP_EEOF            ///< Unexpected end of resource.
#define AH_EEXIST          AHP_EEXIST          ///< Already exists.
#define AH_EFAULT          AHP_EFAULT          ///< Bad pointer.
#define AH_EFBIG           AHP_EFBIG           ///< File too large.
#define AH_EHOSTDOWN       AHP_EHOSTDOWN       ///< Host down.
#define AH_EHOSTUNREACH    AHP_EHOSTUNREACH    ///< Host unreachable.
#define AH_EIDRM           AHP_EIDRM           ///< Identifier removed.
#define AH_EILSEQ          AHP_EILSEQ          ///< Illegal byte sequence.
#define AH_EINPROGRESS     AHP_EINPROGRESS     ///< In progress.
#define AH_EINTERN         AHP_EINTERN         ///< Internal error.
#define AH_EINTR           AHP_EINTR           ///< Interrupted.
#define AH_EINVAL          AHP_EINVAL          ///< Invalid argument.
#define AH_EIO             AHP_EIO             ///< I/O error.
#define AH_EISCONN         AHP_EISCONN         ///< Already connected.
#define AH_EISDIR          AHP_EISDIR          ///< Is a directory.
#define AH_ELOOP           AHP_ELOOP           ///< Too many levels of symbolic links.
#define AH_EMFILE          AHP_EMFILE          ///< File descriptor value too large.
#define AH_EMLINK          AHP_EMLINK          ///< Too many links.
#define AH_EMSGSIZE        AHP_EMSGSIZE        ///< Message too large.
#define AH_EMULTIHOP       AHP_EMULTIHOP       ///< Incomplete route path.
#define AH_ENAMETOOLONG    AHP_ENAMETOOLONG    ///< Name too long.
#define AH_ENETDOWN        AHP_ENETDOWN        ///< Network is down.
#define AH_ENETRESET       AHP_ENETRESET       ///< Connection aborted by network.
#define AH_ENETUNREACH     AHP_ENETUNREACH     ///< Network unreachable.
#define AH_ENFILE          AHP_ENFILE          ///< Too many files open in system.
#define AH_ENOBUFS         AHP_ENOBUFS         ///< No buffer space available.
#define AH_ENODATA         AHP_ENODATA         ///< No data available.
#define AH_ENODEV          AHP_ENODEV          ///< No such device.
#define AH_ENOENT          AHP_ENOENT          ///< No such entry.
#define AH_ENOEXEC         AHP_ENOEXEC         ///< Executable file format error.
#define AH_ENOLCK          AHP_ENOLCK          ///< No locks available.
#define AH_ENOLINK         AHP_ENOLINK         ///< Link severed.
#define AH_ENOMEM          AHP_ENOMEM          ///< Not enough memory.
#define AH_ENOMSG          AHP_ENOMSG          ///< No such message.
#define AH_ENOPROTOOPT     AHP_ENOPROTOOPT     ///< Protocol not available.
#define AH_ENOSPC          AHP_ENOSPC          ///< No space left.
#define AH_ENOSR           AHP_ENOSR           ///< No STREAM resources.
#define AH_ENOSTR          AHP_ENOSTR          ///< Not a STREAM.
#define AH_ENOSYS          AHP_ENOSYS          ///< System call unsupported.
#define AH_ENOTBLK         AHP_ENOTBLK         ///< Not a block device.
#define AH_ENOTCONN        AHP_ENOTCONN        ///< Not connected.
#define AH_ENOTDIR         AHP_ENOTDIR         ///< Not a directory or a symbolic link to a directory.
#define AH_ENOTEMPTY       AHP_ENOTEMPTY       ///< Not empty.
#define AH_ENOTRECOVERABLE AHP_ENOTRECOVERABLE ///< Not recoverable.
#define AH_ENOTSOCK        AHP_ENOTSOCK        ///< Not a socket.
#define AH_ENXIO           AHP_ENXIO           ///< No such device or address.
#define AH_EOPNOTSUPP      AHP_EOPNOTSUPP      ///< Operation not supported.
#define AH_EOVERFLOW       AHP_EOVERFLOW       ///< Value does not fit in target.
#define AH_EOWNERDEAD      AHP_EOWNERDEAD      ///< Previous owner died.
#define AH_EPERM           AHP_EPERM           ///< Not permitted.
#define AH_EPFNOSUPPORT    AHP_EPFNOSUPPORT    ///< Protocol family not supported.
#define AH_EPIPE           AHP_EPIPE           ///< Broken pipe.
#define AH_EPROTO          AHP_EPROTO          ///< Protocol error.
#define AH_EPROTONOSUPPORT AHP_EPROTONOSUPPORT ///< Protocol not supported.
#define AH_EPROTOTYPE      AHP_EPROTOTYPE      ///< Protocol type wrong.
#define AH_ERANGE          AHP_ERANGE          ///< Arithmetic result outside accepted range.
#define AH_EROFS           AHP_EROFS           ///< Read-only file system.
#define AH_ESHUTDOWN       AHP_ESHUTDOWN       ///< Has shut down.
#define AH_ESOCKTNOSUPPORT AHP_ESOCKTNOSUPPORT ///< Socket type not supported.
#define AH_ESPIPE          AHP_ESPIPE          ///< Broken pipe.
#define AH_ESRCH           AHP_ESRCH           ///< Not found.
#define AH_ESTALE          AHP_ESTALE          ///< Stale.
#define AH_ESTATE          AHP_ESTATE          ///< State invalid.
#define AH_ESYNTAX         AHP_ESYNTAX         ///< Syntax invalid.
#define AH_ETIME           AHP_ETIME           ///< STREAM timeout.
#define AH_ETIMEDOUT       AHP_ETIMEDOUT       ///< Timed out.
#define AH_ETOOMANYREFS    AHP_ETOOMANYREFS    ///< Too many references.
#define AH_ETXTBSY         AHP_ETXTBSY         ///< Text file busy.
#define AH_EUSERS          AHP_EUSERS          ///< Too many users.
#define AH_EXDEV           AHP_EXDEV           ///< Cross-device link.
/** @} */

/**
 * Gets string representation of given error code.
 *
 * The representation consists of the name of the error code constant without
 * the @c "AH_" prefix, such as in @c "OK". If the given error code is not one
 * of the ones explicitly listed in err.h, the representation consists of the
 * string @c "ERR[%d]", where @c "%d" is replaced with the actual value of the
 * code on the current platform. Optionally, the string may be followed by a
 * colon and a platform-specific error description.
 *
 * @param[in] err Error code to represent as text.
 *
 * @return Pointer to constant string.
 *
 * @note This function is thread safe on all supported platforms.
 *
 * @warning The constant string returned by this function may be overwritten by
 *          a subsequent call to the same function on the same thread.
 */
ah_inline const char* ah_err_get_s(ah_err_t err)
{
    return ahp_err_get_s(err);
}

#endif
