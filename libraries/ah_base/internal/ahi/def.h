// SPDX-License-Identifier: EPL-2.0

#ifndef AHI_DEF_H
#define AHI_DEF_H

#if defined(__clang__)
# if __clang_major__ < 13
#  warning "Only clang versions 13 and above are supported for this library."
# endif
#elif defined(__GNUC__)
# if __GNUC__ < 10
#  warning "Only GCC versions 10 and above are supported for this library."
# endif
#else
# warning "Only the Clang and GCC compilers are supported for this library."
#endif

#if __has_include("build.gen")
# include "build.gen"
#else
# define AHI_BASE_VERSION       "0.0.0+Unknown.Unknown"
# define AHI_BASE_VERSION_MAJOR 0
# define AHI_BASE_VERSION_MINOR 0
# define AHI_BASE_VERSION_PATCH 0
# define AHI_BASE_COMMIT        "Unknown"
# define AHI_BASE_PLATFORM      "Unknown"
#endif

#define ahi_inline __attribute((unused)) static inline
#define ahi_unused __attribute((unused))

typedef struct ahi_loop_evt ahi_loop_evt_t;

#endif
