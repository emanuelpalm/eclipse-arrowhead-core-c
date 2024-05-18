// SPDX-License-Identifier: EPL-2.0

#ifndef AHP_TIME_H
#define AHP_TIME_H

#include "def.h"

#include <mach/mach_time.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/errno.h>

#define AHP_TIME_ZERO INT64_C(0)

ahp_inline uint64_t ahp_time_now(void)
{
    return mach_absolute_time();
}

ahp_err_t ahp_time_diff(uint64_t a, uint64_t b, int64_t* ns);

ahp_inline int ahp_time_cmp(uint64_t a, uint64_t b)
{
    if (a == b) {
        return 0;
    }
    if (a < b) {
        return -1;
    }
    return 1;
}

ahp_err_t ahp_time_add(uint64_t t, int64_t ns, uint64_t* res);
ahp_err_t ahp_time_sub(uint64_t t, int64_t ns, uint64_t* res);

ahp_inline bool ahp_time_is_after(uint64_t a, uint64_t b)
{
    return a > b;
}

ahp_inline bool ahp_time_is_before(uint64_t a, uint64_t b)
{
    return a < b;
}

ahp_inline bool ahp_time_is_zero(uint64_t t)
{
    return t == INT64_C(0);
}

ahp_err_t ahp_time_to_epoch_ms(uint64_t t, uint64_t* res);

ahp_err_t ahp_epoch_get_ms(uint64_t* res);
ahp_err_t ahp_epoch_set_ms(uint64_t epoch_ms);

#endif
