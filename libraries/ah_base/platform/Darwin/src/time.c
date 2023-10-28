// SPDX-License-Identifier: EPL-2.0

#include "ahp/time.h"

#include "ahp/err.h"

#include <stdio.h>
#include <stdlib.h>

static mach_timebase_info_data_t ahi_get_info(void);
static bool ahi_mul_div_overflow(int64_t a, int64_t b, int64_t c, int64_t* res);

ahp_err_t ahp_time_diff(uint64_t a, uint64_t b, int64_t* ns)
{
    if (ns == NULL) {
        return AHP_EINVAL;
    }

    int64_t d;
    if (__builtin_sub_overflow(a, b, &d)) {
        return AHP_ERANGE;
    }

    mach_timebase_info_data_t info = ahi_get_info();

    if (ahi_mul_div_overflow(d, info.numer, info.denom, ns)) {
        return AHP_ERANGE;
    }

    return AHP_OK;
}

ahp_err_t ahp_time_add(uint64_t t, int64_t ns, uint64_t* res)
{
    if (res == NULL) {
        return AHP_EINVAL;
    }

    mach_timebase_info_data_t info = ahi_get_info();

    int64_t u;
    if (ahi_mul_div_overflow(ns, info.denom, info.numer, &u)) {
        return AHP_ERANGE;
    }
    if (__builtin_add_overflow(t, u, res)) {
        return AHP_ERANGE;
    }

    return AHP_OK;
}

ahp_err_t ahp_time_sub(uint64_t t, int64_t ns, uint64_t* res)
{
    if (res == NULL) {
        return AHP_EINVAL;
    }

    mach_timebase_info_data_t info = ahi_get_info();

    int64_t u;
    if (ahi_mul_div_overflow(ns, info.denom, info.numer, &u)) {
        return AHP_ERANGE;
    }
    if (__builtin_sub_overflow(t, u, res)) {
        return AHP_ERANGE;
    }

    return AHP_OK;
}

static mach_timebase_info_data_t ahi_get_info(void)
{
    mach_timebase_info_data_t info;
    kern_return_t res = mach_timebase_info(&info);
    if (res == KERN_SUCCESS) {
        return info;
    }

    (void) fprintf(stderr,
        "mach_timebase_info(&info) failed; returned %d.",
        res);

    abort();
}

static bool ahi_mul_div_overflow(int64_t a, int64_t b, int64_t c, int64_t* res)
{
    int64_t q = a / c;
    int64_t r = a % c;

    int64_t qb;
    if (__builtin_mul_overflow(q, b, &qb)) {
        return true;
    }
    if (__builtin_add_overflow(qb, r * b / c, res)) {
        return true;
    }

    return false;
}
