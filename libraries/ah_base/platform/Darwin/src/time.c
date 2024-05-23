// SPDX-License-Identifier: EPL-2.0

#include "ahp/time.h"

#include "ahp/err.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define AHI_EPOCH_BASE_MS_UNSET UINT64_MAX

static ahp_err_t ahi_get_epoch_base_ms(mach_timebase_info_data_t* info, uint64_t* epoch_base_ms);
static mach_timebase_info_data_t ahi_get_timebase_info(void);

static int64_t ahi_mul_div_i64(int64_t a, int64_t b, int64_t c);
static uint64_t ahi_mul_div_u64(uint64_t a, uint64_t b, uint64_t c);

static uint64_t AHI_EPOCH_BASE_MS = AHI_EPOCH_BASE_MS_UNSET;

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

ahp_err_t ahp_time_to_epoch_ms(uint64_t t, uint64_t* res)
{
    if (res == NULL) {
        return AHP_EINVAL;
    }

    mach_timebase_info_data_t info = ahi_get_timebase_info();

    uint64_t epoch_base_ms;
    epoch_base_ms = __atomic_load_n(&AHI_EPOCH_BASE_MS, __ATOMIC_RELAXED);
    if (epoch_base_ms == AHI_EPOCH_BASE_MS_UNSET) {
        ahp_err_t err = ahi_get_epoch_base_ms(&info, &epoch_base_ms);
        if (err != AHP_OK) {
            return err;
        }
        __atomic_store_n(&AHI_EPOCH_BASE_MS, epoch_base_ms, __ATOMIC_RELAXED);
    }

    uint64_t t_ms = ahi_mul_div_u64(t, info.numer, info.denom) / 1000000u;

    if (__builtin_add_overflow(epoch_base_ms, t_ms, res)) {
        return AHP_ERANGE;
    }

    return AHP_OK;
}

ahp_err_t ahp_epoch_get_ms(uint64_t* res)
{
    return ahp_time_to_epoch_ms(ahp_time_now(), res);
}

__attribute((cold)) ahp_err_t ahp_epoch_set_ms(uint64_t epoch_ms)
{
    uint64_t at = mach_absolute_time();

    mach_timebase_info_data_t info = ahi_get_timebase_info();
    uint64_t at_ms = ahi_mul_div_u64(at, info.denom, info.numer) / 1000000u;

    uint64_t epoch_base_ms;
    if (__builtin_sub_overflow(epoch_ms, at_ms, &epoch_base_ms)) {
        return AHP_ECLOCKRANGE;
    }

    __atomic_store_n(&AHI_EPOCH_BASE_MS, epoch_base_ms, __ATOMIC_RELAXED);

    return AHP_OK;
}

static __attribute((cold)) ahp_err_t ahi_get_epoch_base_ms(mach_timebase_info_data_t* info,
    uint64_t* epoch_base_ms)
{
    struct timespec rt;
    if (clock_gettime(CLOCK_REALTIME, &rt) == -1) {
        (void) fprintf(stderr, "clock_gettime(CLOCK_REALTIME) failed; %s.", strerror(errno));

        abort();
    }
    uint64_t at = mach_absolute_time();

    uint64_t rt_ms;
    if (__builtin_mul_overflow(rt.tv_sec, 1000u, &rt_ms)) {
        return AHP_ECLOCKRANGE;
    }
    if (__builtin_add_overflow(rt_ms, rt.tv_nsec / 1000000u, &rt_ms)) {
        return AHP_ECLOCKRANGE;
    }

    uint64_t at_ms = ahi_mul_div_u64(at, info->numer, info->denom) / 1000000u;

    if (__builtin_sub_overflow(rt_ms, at_ms, epoch_base_ms)) {
        return AHP_ECLOCKRANGE;
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

#define IMPL_MUL_DIV(NAME, TYPE)             \
    static TYPE NAME(TYPE a, TYPE b, TYPE c) \
    {                                        \
        TYPE q = a / c;                      \
        TYPE r = a % c;                      \
        TYPE qb = q * b;                     \
        return qb + r * b / c;               \
    }

IMPL_MUL_DIV(ahi_mul_div_i64, int64_t)
IMPL_MUL_DIV(ahi_mul_div_u64, uint64_t)
