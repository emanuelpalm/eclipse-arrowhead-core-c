// SPDX-License-Identifier: EPL-2.0

#ifndef AH_CKDINT_H
#define AH_CKDINT_H

#include <ahi/ckdint.h>

/**
 * @file
 * Checked integer arithmetics.
 *
 * Special builtin functions that allows for arithmetic overflows to be
 * detected efficiently.
 */

/**
 * Adds @a a to @a b and stores the result to @a res.
 *
 * Conceptually, @a a and @a b are both promoted into an infinite precision
 * signed integer type before the operation is performed. The result is then
 * cast to the type pointed at by @a res. If the result falls outside the range
 * of values that can be expressed by that type, the state of @a res is
 * undefined and the function returns @c true. If, on the other hand, the cast
 * is successful @a res will contain the result and @c false is returned.
 *
 * @param[out] res Pointer to a signed or unsigned integer type that will
 *                 receive the result of the operation, if it is successful.
 * @param[in]  a   The augend, which may be of any integer type.
 * @param[in]  b   The addend, which may be of any integer type.
 *
 * @return @c true only if the operation overflowed. @c false otherwise.
 *
 * @warning The state of @a res is undefined after the function returns unless
 *          its return value is @c false.
 */
#define ah_ckd_add(res, a, b) ahi_ckd_add((res), (a), (b))

/**
 * Multiplies @a a with @a b and stores the result to @a res.
 *
 * Conceptually, @a a and @a b are both promoted into an infinite precision
 * signed integer type before the operation is performed. The result is then
 * cast to the type pointed at by @a res. If the result falls outside the range
 * of values that can be expressed by that type, the state of @a res is
 * undefined and the function returns @c true. If, on the other hand, the cast
 * is successful @a res will contain the result and @c false is returned.
 *
 * @param[out] res Pointer to a signed or unsigned integer type that will
 *                 receive the result of the operation, if it is successful.
 * @param[in]  a   The multiplier, which may be of any integer type.
 * @param[in]  b   The multiplicand, which may be of any integer type.
 *
 * @return @c true only if the operation overflowed. @c false otherwise.
 *
 * @warning The state of @a res is undefined after the function returns unless
 *          its return value is @c false.
 */
#define ah_ckd_mul(res, a, b) ahi_ckd_mul((res), (a), (b))

/**
 * Subtracts @a a and @a b and stores the result to @a res.
 *
 * Conceptually, @a a and @a b are both promoted into an infinite precision
 * signed integer type before the operation is performed. The result is then
 * cast to the type pointed at by @a res. If the result falls outside the range
 * of values that can be expressed by that type, the state of @a res is
 * undefined and the function returns @c true. If, on the other hand, the cast
 * is successful @a res will contain the result and @c false is returned.
 *
 * @param[out] res Pointer to a signed or unsigned integer type that will
 *                 receive the result of the operation, if it is successful.
 * @param[in]  a   The minuend, which may be of any integer type.
 * @param[in]  b   The subtrahend, which may be of any integer type.
 *
 * @return @c true only if the operation overflowed. @c false otherwise.
 *
 * @warning The state of @a res is undefined after the function returns unless
 *          its return value is @c false.
 */
#define ah_ckd_sub(res, a, b) ahi_ckd_sub((res), (a), (b))

#endif
