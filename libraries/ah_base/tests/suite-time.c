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

    AH_UNIT_TEST("ah_time_add() rejects invalid arguments.")
    {
        AH_UNIT_CASE("`res` is NULL.")
        {
            AH_UNIT_EQ_ERR(AH_EINVAL, ah_time_add(AH_TIME_ZERO, 0, NULL));
        }

        AH_UNIT_CASE("`t` added to `ns` overflows.")
        {
            ah_time_t t;
            AH_UNIT_EQ_ERR(AH_ERANGE, ah_time_add(AH_TIME_MAX, 1000000, &t));
        }

        AH_UNIT_CASE("`t` added to `ns` underflows.")
        {
            ah_time_t t;
            AH_UNIT_EQ_ERR(AH_ERANGE, ah_time_add(AH_TIME_MIN, -1000000, &t));
        }
    }

    AH_UNIT_TEST("ah_time_add() adds time and duration as expected.")
    {
        ah_time_t t;
        AH_UNIT_EQ_ERR(AH_OK, ah_time_add(AH_TIME_ZERO, 12340 * NS_PER_MS, &t));

        ah_time_t actual;
        AH_UNIT_EQ_ERR(AH_OK, ah_time_add(t, 5 * NS_PER_MS, &actual));

        ah_time_t expected;
        AH_UNIT_EQ_ERR(AH_OK, ah_time_add(AH_TIME_ZERO, 12345 * NS_PER_MS, &expected));

        AH_UNIT_EQ_INT(0, ah_time_cmp(expected, actual));
    }

    AH_UNIT_TEST("ah_time_sub() rejects invalid arguments.")
    {
        AH_UNIT_CASE("`res` is NULL.")
        {
            AH_UNIT_EQ_ERR(AH_EINVAL, ah_time_sub(AH_TIME_ZERO, 0, NULL));
        }

        AH_UNIT_CASE("`t` added to `ns` overflows.")
        {
            ah_time_t t;
            AH_UNIT_EQ_ERR(AH_ERANGE, ah_time_sub(AH_TIME_MIN, 1000000, &t));
        }

        AH_UNIT_CASE("`t` added to `ns` underflows.")
        {
            ah_time_t t;
            AH_UNIT_EQ_ERR(AH_ERANGE, ah_time_sub(AH_TIME_MAX, -1000000, &t));
        }
    }

    AH_UNIT_TEST("ah_time_sub() adds time and duration as expected.")
    {
        ah_time_t t;
        AH_UNIT_EQ_ERR(AH_OK, ah_time_add(AH_TIME_ZERO, 12345 * NS_PER_MS, &t));

        ah_time_t actual;
        AH_UNIT_EQ_ERR(AH_OK, ah_time_sub(t, 5 * NS_PER_MS, &actual));

        ah_time_t expected;
        AH_UNIT_EQ_ERR(AH_OK, ah_time_add(AH_TIME_ZERO, 12340 * NS_PER_MS, &expected));

        AH_UNIT_EQ_INT(0, ah_time_cmp(expected, actual));
    }

    AH_UNIT_TEST("ah_time_is_{after,before}() behaves as expected.")
    {
        AH_UNIT_CASE("a > b")
        {
            ah_time_t a;
            AH_UNIT_EQ_ERR(AH_OK, ah_time_add(AH_TIME_ZERO, 2 * NS_PER_MS, &a));

            ah_time_t b;
            AH_UNIT_EQ_ERR(AH_OK, ah_time_add(AH_TIME_ZERO, 1 * NS_PER_MS, &b));

            AH_UNIT_EQ_BOOL(true, ah_time_is_after(a, b));
            AH_UNIT_EQ_BOOL(false, ah_time_is_before(a, b));
        }

        AH_UNIT_CASE("a == b")
        {
            ah_time_t a;
            AH_UNIT_EQ_ERR(AH_OK, ah_time_add(AH_TIME_ZERO, 3 * NS_PER_MS, &a));

            ah_time_t b;
            AH_UNIT_EQ_ERR(AH_OK, ah_time_add(AH_TIME_ZERO, 3 * NS_PER_MS, &b));

            AH_UNIT_EQ_BOOL(false, ah_time_is_after(a, b));
            AH_UNIT_EQ_BOOL(false, ah_time_is_before(a, b));
        }

        AH_UNIT_CASE("a < b")
        {
            ah_time_t a;
            AH_UNIT_EQ_ERR(AH_OK, ah_time_add(AH_TIME_ZERO, 4 * NS_PER_MS, &a));

            ah_time_t b;
            AH_UNIT_EQ_ERR(AH_OK, ah_time_add(AH_TIME_ZERO, 5 * NS_PER_MS, &b));

            AH_UNIT_EQ_BOOL(false, ah_time_is_after(a, b));
            AH_UNIT_EQ_BOOL(true, ah_time_is_before(a, b));
        }
    }

    AH_UNIT_TEST("ah_time_is_zero() returns true if `t` is zeroed.")
    {
        ah_time_t t;
        memset(&t, 0u, sizeof(t));

        AH_UNIT_EQ_BOOL(true, ah_time_is_zero(t));
    }

    AH_UNIT_TEST("ah_time_is_zero() returns true if `t` is AH_TIME_ZERO.")
    {
        AH_UNIT_EQ_BOOL(true, ah_time_is_zero(AH_TIME_ZERO));
    }

    AH_UNIT_TEST("ah_time_is_zero() returns false if `t` is not zeroed or AH_TIME_ZERO.")
    {
        ah_time_t t = AH_TIME_ZERO;
        AH_UNIT_EQ_ERR(AH_OK, ah_time_add(AH_TIME_ZERO, 4 * NS_PER_MS, &t));

        AH_UNIT_EQ_BOOL(false, ah_time_is_zero(t));
    }

    AH_UNIT_TEST("ah_time_to_epoch_ms() rejects invalid arguments.")
    {
        AH_UNIT_CASE("`res` is NULL.")
        {
            AH_UNIT_EQ_ERR(AH_EINVAL, ah_time_to_epoch_ms(AH_TIME_ZERO, NULL));
        }
    }
}
