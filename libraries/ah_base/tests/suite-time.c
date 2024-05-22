// SPDX-License-Identifier: EPL-2.0

#include "ah/time.h"

#include <ah/unit.h>

AH_UNIT_SUITE(time)
{
    const int64_t NS_PER_MS = 1000000;

    AH_UNIT_TEST("ah_time_now() does not return a time equal to AH_TIME_ZERO.")
    {
        ah_time_t a = AH_TIME_ZERO;
        ah_time_t b = ah_time_now();

        AH_UNIT_NE_MEM(&a, &b, sizeof(ah_time_t));
    }

    AH_UNIT_TEST("ah_time_diff() produces result with at least millisecond precision.")
    {
        ah_time_t a;
        AH_UNIT_EQ_ERR(AH_OK, ah_time_add(AH_TIME_ZERO, 451 * NS_PER_MS, &a));

        ah_time_t b;
        AH_UNIT_EQ_ERR(AH_OK, ah_time_add(AH_TIME_ZERO, 1025 * NS_PER_MS, &b));

        int64_t d;
        AH_UNIT_EQ_ERR(AH_OK, ah_time_diff(a, b, &d));
        AH_UNIT_EQ_INT(-574, d / NS_PER_MS);
    }

    AH_UNIT_TEST("ah_time_cmp() behaves as expected.")
    {
        AH_UNIT_CASE("a > b")
        {
            ah_time_t a;
            AH_UNIT_EQ_ERR(AH_OK, ah_time_add(AH_TIME_ZERO, 2 * NS_PER_MS, &a));

            ah_time_t b;
            AH_UNIT_EQ_ERR(AH_OK, ah_time_add(AH_TIME_ZERO, 1 * NS_PER_MS, &b));

            AH_UNIT_LT_INT(0, ah_time_cmp(a, b));
        }

        AH_UNIT_CASE("a == b")
        {
            ah_time_t a;
            AH_UNIT_EQ_ERR(AH_OK, ah_time_add(AH_TIME_ZERO, 3 * NS_PER_MS, &a));

            ah_time_t b;
            AH_UNIT_EQ_ERR(AH_OK, ah_time_add(AH_TIME_ZERO, 3 * NS_PER_MS, &b));

            AH_UNIT_EQ_INT(0, ah_time_cmp(a, b));
        }

        AH_UNIT_CASE("a < b")
        {
            ah_time_t a;
            AH_UNIT_EQ_ERR(AH_OK, ah_time_add(AH_TIME_ZERO, 4 * NS_PER_MS, &a));

            ah_time_t b;
            AH_UNIT_EQ_ERR(AH_OK, ah_time_add(AH_TIME_ZERO, 5 * NS_PER_MS, &b));

            AH_UNIT_GT_INT(0, ah_time_cmp(a, b));
        }
    }
}
