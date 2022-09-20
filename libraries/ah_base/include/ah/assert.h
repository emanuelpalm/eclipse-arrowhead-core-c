// SPDX-License-Identifier: EPL-2.0

#ifndef AH_ASSERT_H_
#define AH_ASSERT_H_

/**
 * @file
 * Assertion macros.
 *
 * Provides macros analogous to the C99 @c assert() macro. The key differences
 * being, however, that a variant exists that is not stripped when compiling
 * with @c -DNDEBUG and that ah_abortf() is used to report the result of failed
 * assertions.
 */
#include "abort.h"
#include "intrin.h"

/**
 * Aborts application if @a expr evaluates to false.
 *
 * @param expr Arbitrary expression.
 *
 * @note Invocations of this macro will @e not be removed if compiling with
 * @c -DNDEBUG. If you do want certain of them to be removed in that case,
 * please make use of ah_assert_if_debug() instead.
 *
 * @see ah_abort()
 */
#define ah_assert_always(expr) (ah_likely((expr)) ? ((void) 0) : ah_abortf("%s:%d " #expr "\n", __FILE__, __LINE__))

#ifndef NDEBUG

/**
 * Aborts application if @a expr evaluates to false, unless the application is
 * compiled with @c -DNDEBUG.
 *
 * @param expr Arbitrary expression.
 *
 * @note Invocations of this macro @e will be removed if compiling with
 * @c -DNDEBUG. If you do not want certain of them to be removed in that case,
 * please make use of ah_assert_always() instead.
 *
 * @see ah_abort()
 */
# define ah_assert_if_debug(expr) ah_assert_always(expr)

#else
# define ah_assert_if_debug(expr) ((void) 0)
#endif

#endif
