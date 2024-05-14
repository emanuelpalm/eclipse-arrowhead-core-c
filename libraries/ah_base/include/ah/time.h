// SPDX-License-Identifier: EPL-2.0

#ifndef AH_TIME_H
#define AH_TIME_H

/**
 * @file
 * Time querying and comparison.
 *
 * Functions for querying a platform clock and comparing its outputs.
 */

#include "def.h"

#include <ahp/time.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/**
 * The zero time.
 *
 * Expands to an initializer or value that can be used to initialize an
 * ah_time_t variable to zero.
 */
#define AH_TIME_ZERO AHP_TIME_ZERO

/**
 * Gets the current time, as reported by the platform.
 *
 * What API is consulted to get the time varies with the targeted platform, as
 * outlined in the following table:
 *
 * <table>
 *   <tr>
 *     <th>Platform
 *     <th>API
 *   <tr>
 *     <td>Darwin
 *     <td><a href="https://developer.apple.com/documentation/kernel/1462446-mach_absolute_time">mach_absolute_time()</a>
 *   <tr>
 *     <td>Linux
 *     <td><a href="https://linux.die.net/man/3/clock_gettime">clock_gettime(CLOCK_MONOTONIC)</a>
 *   <tr>
 *     <td>Windows
 *     <td><a href="https://docs.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancecounter">QueryPerformanceCounter()</a>
 * </table>
 *
 * @return Current time.
 */
ah_inline ah_time_t ah_time_now(void)
{
    return ahp_time_now();
}

/**
 * Calculates the nanosecond difference between the instants @a a and @a b.
 *
 * @param[in]  a  First time.
 * @param[in]  b  Second time.
 * @param[out] ns Pointer to receiver of nanosecond difference between @a a and
 *                @a b.
 *
 * @retval AH_OK     if successful.
 * @retval AH_EINVAL if @a diff is @c NULL.
 * @retval AH_ERANGE if @a a minus @a b produced an unrepresentable result.
 */
ah_inline ah_err_t ah_time_diff(ah_time_t a, ah_time_t b, int64_t* ns)
{
    return ahp_time_diff(a, b, ns);
}

/**
 * Compares @a a to @a b.
 *
 * @param a First time.
 * @param b Second time.
 *
 * @return An an integer greater than, equal to, or less than 0, depending on
 *         if @a a is greater than, equal to, or less than @a b.
 */
ah_inline int ah_time_cmp(ah_time_t a, ah_time_t b)
{
    return ahp_time_cmp(a, b);
}

/**
 * Increases @a time by @a ns, storing the result to @a res.
 *
 * @param t   Instant to increase.
 * @param ns  Number of nanoseconds to increase @a t with.
 * @param res Pointer to receiver of the result.
 *
 * @retval AH_OK     if successful.
 * @retval AH_EINVAL if @a res is @c NULL.
 * @retval AH_ERANGE if @a t plus @a ns produced an unrepresentable result.
 */
ah_inline ah_err_t ah_time_add(ah_time_t t, int64_t ns, ah_time_t* res)
{
    return ahp_time_add(t, ns, res);
}

/**
 * Decreases @a time by @a ns, storing the result to @a res.
 *
 * @param t   Instant to decrease.
 * @param ns  Number of nanoseconds to decrease @a t with.
 * @param res Pointer to receiver of the result.
 *
 * @retval AH_OK     if successful.
 * @retval AH_EINVAL if @a res is @c NULL.
 * @retval AH_ERANGE if @a t minus @a ns produced an unrepresentable result.
 */
ah_inline ah_err_t ah_time_sub(ah_time_t t, int64_t ns, ah_time_t* res)
{
    return ahp_time_sub(t, ns, res);
}

/**
 * Checks if @a a represents a point in time after @a b.
 *
 * @param a First time.
 * @param b Second time.
 *
 * @return @c true only if @a a occurs after @a b. @c false otherwise.
 */
ah_inline bool ah_time_is_after(ah_time_t a, ah_time_t b)
{
    return ahp_time_is_after(a, b);
}

/**
 * Checks if @a a represents a point in time before @a b.
 *
 * @param a First time.
 * @param b Second time.
 *
 * @return @c true only if @a a occurs before @a b. @c false otherwise.
 */
ah_inline bool ah_time_is_before(ah_time_t a, ah_time_t b)
{
    return ahp_time_is_before(a, b);
}

/**
 * Checks if @a t is the zero time.
 *
 * @param time Time.
 * @return @c true only if @a t is zeroed.
 *
 * @note The zero time can be produced by setting the memory of an @c ah_time_t
 * instance to all zeroes, or by using the @c AH_TIME_ZERO macro.
 */
ah_inline bool ah_time_is_zero(ah_time_t t)
{
    return ahp_time_is_zero(t);
}

#endif
