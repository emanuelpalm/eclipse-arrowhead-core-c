// SPDX-License-Identifier: EPL-2.0

#ifndef AH_DEF_H
#define AH_DEF_H

#include <ahi/def.h>
#include <ahp/def.h>

/**
 * @file
 * Generally useful definitions.
 *
 * Generally significant macro constants, macro attributes, type forward
 * declarations and typedefs.
 */

/**
 * @name Attribute Macros
 *
 * Macro constants used as attributes that can be placed before declarations.
 *
 * @{
 */

/**
 * A function definition following this attribute is inlined by the compiler.
 */
#define ah_inline ahi_inline

/**
 * A declaration following this attribute may be unused.
 */
#define ah_unused ahi_unused

/** @} */

/**
 * @name Platform Constants
 *
 * Macro constants defined only if they identify the targeted platform.
 *
 * @{
 */

#ifdef AH_DOCS

/**
 * Defined only if the target platform is Darwin.
 *
 * Darwin is the name of the kernel that makes up the core of the operating
 * systems of Apple, such as macOS, iOS and watchOS.
 */
# define AH_DARWIN

/**
 * Defined only if the targeted platform is Linux.
 */
# define AH_LINUX

/**
 * Defined only if the targeted platform is Windows.
 */
# define AH_WINDOWS

#endif

/** @} */

/**
 * @name Version Constants
 *
 * Macro constants describing the current version of this library.
 *
 * @{
 */

/**
 * The version of this library, as a string literal.
 *
 * This string will contain the major, minor and patch versions of the library,
 * as well as AH_BASE_COMMIT and AH_BASE_PLATFORM. The string will appear as if
 * it was produced via the mentioned values, in the order they were presented,
 * and the format string @c "%d.%d.%d+%s.%s". The format string must be
 * interpreted as if it is the format argument to the C @c printf() function.
 */
#define AH_BASE_VERSION AHI_BASE_VERSION

/**
 * The major version of this library, as an integer constant.
 *
 * Every time a backwards-incompatible change is made to the API of this
 * library, the major version is incremented and the minor and patch versions
 * set to @c 0.
 */
#define AH_BASE_VERSION_MAJOR AHI_BASE_VERSION_MAJOR

/**
 * The minor version of this library, as an integer constant.
 *
 * Every time a backwards-compatible change is made to the API of this library,
 * the minor version is incremented and the patch version is set to @c 0.
 */
#define AH_BASE_VERSION_MINOR AHI_BASE_VERSION_MINOR

/**
 * The patch version of this library, as an integer constant.
 *
 * The patch version is incremented when changes are made to the library that
 * does not affect its API.
 */
#define AH_BASE_VERSION_PATCH AHI_BASE_VERSION_PATCH

/**
 * The version control system commit of this library, as a string literal.
 */
#define AH_BASE_COMMIT AHI_BASE_COMMIT

/**
 * The name of the platform for which this library was built, as a string
 * literal.
 */
#define AH_BASE_PLATFORM AHI_BASE_PLATFORM

/** @} */

/**
 * @name Platform Type Aliases
 *
 * Various types exposed by the underlying platform. Unless otherwise is noted,
 * applications that are meant to work across multiple platforms should not
 * access the values or members of these types directly. Rather should the
 * functions provided by this library for dealing with these types be used.
 *
 * @{
 */

/**
 * A buffer, consisting of a pointer to a memory region and its size, in bytes.
 */
typedef ahp_buf_t ah_buf_t;

/**
 * An error code.
 *
 * While no guarantees are made about the size or signedness of this type, it
 * is guaranteed to be an integer type. It is always safe to directly compare a
 * value of this type to the error code constants listed in err.h.
 */
typedef ahp_err_t ah_err_t;

/**
 * A point in time, relative to some arbitrary point in the past.
 */
typedef ahp_time_t ah_time_t;

/** @} */

/**
 * @name Type Forward Declarations
 *
 * Forward declarations with typedefs for all types declared in this library.
 *
 * @{
 */

typedef struct ah_bump ah_bump_t;
typedef struct ah_bufc ah_bufc_t;
typedef struct ah_slab ah_slab_t;

/** @} */

#endif
