// SPDX-License-Identifier: EPL-2.0

#include "ahp/time.h"

#include "ahp/err.h"

#include <stdio.h>
#include <stdlib.h>

static mach_timebase_info_data_t ahi_get_timebase_info(void);
static int64_t ahi_mul_div_i64(int64_t a, int64_t b, int64_t c);


ahp_err_t ahp_time_diff(uint64_t a, uint64_t b, int64_t* ns)
{
    if (ns == NULL) {
        return AHP_EINVAL;
    }

    int64_t d;
    if (__builtin_sub_overflow(a, b, &d)) {
        return AHP_ERANGE;
    }

    mach_timebase_info_data_t info = ahi_get_timebase_info();
    *ns = ahi_mul_div_i64(d, info.numer, info.denom);

    return AHP_OK;
}

ahp_err_t ahp_time_add(uint64_t t, int64_t ns, uint64_t* res)
{
    if (res == NULL) {
        return AHP_EINVAL;
    }

    mach_timebase_info_data_t info = ahi_get_timebase_info();
    int64_t u = ahi_mul_div_i64(ns, info.denom, info.numer);

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

    mach_timebase_info_data_t info = ahi_get_timebase_info();
    int64_t u = ahi_mul_div_i64(ns, info.denom, info.numer);

    if (__builtin_sub_overflow(t, u, res)) {
        return AHP_ERANGE;
    }

    return AHP_OK;
}

static mach_timebase_info_data_t ahi_get_timebase_info(void)
{
    mach_timebase_info_data_t info;
    kern_return_t res = mach_timebase_info(&info);
    if (res == KERN_SUCCESS) {
        return info;
    }

    (void) fprintf(stderr, "mach_timebase_info() failed; returned %d.", res);

    abort();
}

#define IMPL_MUL_DIV(NAME, TYPE)                                               \
    static TYPE NAME(TYPE a, TYPE b, TYPE c)                                   \
    {                                                                          \
        TYPE q = a / c;                                                        \
        TYPE r = a % c;                                                        \
        TYPE qb = q * b;                                                       \
        return qb + r * b / c;                                                 \
    }

IMPL_MUL_DIV(ahi_mul_div_i64, int64_t)
