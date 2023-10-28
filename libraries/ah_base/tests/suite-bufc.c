// SPDX-License-Identifier: EPL-2.0

#include "ah/buf.h"

#include <ah/unit.h>

AH_UNIT_SUITE(bufc)
{
    uint8_t buffer[32u] = { 0u };
    ah_bufc_t c;
    ah_err_t err;

    AH_UNIT_TEST("ah_bufc_from_readable() rejects invalid arguments.")
    {
        AH_UNIT_CASE("`base` is NULL.")
        {
            c = ah_bufc_from_readable(NULL, sizeof(buffer));
            AH_UNIT_EQ_PTR(NULL, c.r);
            AH_UNIT_EQ_PTR(NULL, c.w);
            AH_UNIT_EQ_PTR(NULL, c.e);
        }

        AH_UNIT_CASE("`size` is 0u.")
        {
            c = ah_bufc_from_readable(buffer, 0u);
            AH_UNIT_EQ_PTR(NULL, c.r);
            AH_UNIT_EQ_PTR(NULL, c.w);
            AH_UNIT_EQ_PTR(NULL, c.e);
        }
    }

    AH_UNIT_TEST("ah_bufc_from_readable() accepts valid arguments.")
    {
        c = ah_bufc_from_readable(buffer, sizeof(buffer));
        AH_UNIT_EQ_PTR(buffer, c.r);
        AH_UNIT_EQ_PTR(&buffer[sizeof(buffer)], c.w);
        AH_UNIT_EQ_PTR(&buffer[sizeof(buffer)], c.e);
    }

    AH_UNIT_TEST("ah_bufc_from_readable_buf() rejects `b` being NULL.")
    {
        c = ah_bufc_from_readable_buf(NULL);
        AH_UNIT_EQ_PTR(NULL, c.r);
        AH_UNIT_EQ_PTR(NULL, c.w);
        AH_UNIT_EQ_PTR(NULL, c.e);
    }

    AH_UNIT_TEST("ah_bufc_from_readable_buf() accepts valid arguments.")
    {
        ah_buf_t b;
        err = ah_buf_init(&b, buffer, sizeof(buffer));
        if (AH_UNIT_EQ_ERR(AH_OK, err)) {
            c = ah_bufc_from_readable_buf(&b);
            AH_UNIT_EQ_PTR(buffer, c.r);
            AH_UNIT_EQ_PTR(&buffer[sizeof(buffer)], c.w);
            AH_UNIT_EQ_PTR(&buffer[sizeof(buffer)], c.e);
        }
    }

    AH_UNIT_TEST("ah_bufc_from_writable() rejects invalid arguments.")
    {
        AH_UNIT_CASE("`base` is NULL.")
        {
            c = ah_bufc_from_writable(NULL, sizeof(buffer));
            AH_UNIT_EQ_PTR(NULL, c.r);
            AH_UNIT_EQ_PTR(NULL, c.w);
            AH_UNIT_EQ_PTR(NULL, c.e);
        }

        AH_UNIT_CASE("`size` is 0u.")
        {
            c = ah_bufc_from_writable(buffer, 0u);
            AH_UNIT_EQ_PTR(NULL, c.r);
            AH_UNIT_EQ_PTR(NULL, c.w);
            AH_UNIT_EQ_PTR(NULL, c.e);
        }
    }

    AH_UNIT_TEST("ah_bufc_from_writable() accepts valid arguments.")
    {
        c = ah_bufc_from_writable(buffer, sizeof(buffer));
        AH_UNIT_EQ_PTR(buffer, c.r);
        AH_UNIT_EQ_PTR(buffer, c.w);
        AH_UNIT_EQ_PTR(&buffer[sizeof(buffer)], c.e);
    }

    AH_UNIT_TEST("ah_bufc_from_writable_buf() rejects `b` being NULL.")
    {
        c = ah_bufc_from_writable_buf(NULL);
        AH_UNIT_EQ_PTR(NULL, c.r);
        AH_UNIT_EQ_PTR(NULL, c.w);
        AH_UNIT_EQ_PTR(NULL, c.e);
    }

    AH_UNIT_TEST("ah_bufc_from_writable_buf() accepts valid arguments.")
    {
        ah_buf_t b;
        err = ah_buf_init(&b, buffer, sizeof(buffer));
        if (AH_UNIT_EQ_ERR(AH_OK, err)) {
            c = ah_bufc_from_writable_buf(&b);
            AH_UNIT_EQ_PTR(buffer, c.r);
            AH_UNIT_EQ_PTR(buffer, c.w);
            AH_UNIT_EQ_PTR(&buffer[sizeof(buffer)], c.e);
        }
    }

    AH_UNIT_TEST("ah_bufc_get_readable_sz() returns 0u if `c` is NULL.")
    {
        AH_UNIT_EQ_UINT(0u, ah_bufc_get_readable_sz(NULL));
    }

    AH_UNIT_TEST("ah_bufc_get_readable_sz() returns correct size.")
    {
        c = ah_bufc_from_readable(buffer, sizeof(buffer));
        AH_UNIT_EQ_UINT(sizeof(buffer), ah_bufc_get_readable_sz(&c));
    }

    AH_UNIT_TEST("ah_bufc_get_writable_sz() returns 0u if `c` is NULL.")
    {
        AH_UNIT_EQ_UINT(0u, ah_bufc_get_writable_sz(NULL));
    }

    AH_UNIT_TEST("ah_bufc_get_writable_sz() returns correct size.")
    {
        c = ah_bufc_from_writable(buffer, sizeof(buffer));
        AH_UNIT_EQ_UINT(sizeof(buffer), ah_bufc_get_writable_sz(&c));
    }

    AH_UNIT_TEST("ah_bufc_is_readable() returns false if `c` is NULL.")
    {
        AH_UNIT_EQ_UINT(false, ah_bufc_is_readable(NULL));
    }

    AH_UNIT_TEST("ah_bufc_is_readable() returns false if not readable.")
    {
        c = ah_bufc_from_writable(buffer, sizeof(buffer));
        AH_UNIT_EQ_BOOL(false, ah_bufc_is_readable(&c));
    }

    AH_UNIT_TEST("ah_bufc_is_readable() returns true if readable.")
    {
        c = ah_bufc_from_readable(buffer, sizeof(buffer));
        AH_UNIT_EQ_BOOL(true, ah_bufc_is_readable(&c));
    }

    AH_UNIT_TEST("ah_bufc_is_writable() returns false if `c` is NULL.")
    {
        AH_UNIT_EQ_UINT(false, ah_bufc_is_writable(NULL));
    }

    AH_UNIT_TEST("ah_bufc_is_writable() returns false if not writable.")
    {
        c = ah_bufc_from_readable(buffer, sizeof(buffer));
        AH_UNIT_EQ_BOOL(false, ah_bufc_is_writable(&c));
    }

    AH_UNIT_TEST("ah_bufc_is_writable() returns true if writable.")
    {
        c = ah_bufc_from_writable(buffer, sizeof(buffer));
        AH_UNIT_EQ_BOOL(true, ah_bufc_is_writable(&c));
    }
}
