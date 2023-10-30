// SPDX-License-Identifier: EPL-2.0

#include "ah/buf.h"

#include <ah/unit.h>

AH_UNIT_SUITE(buf)
{
    uint8_t buffer[32u] = { 0u };
    ah_buf_t b;
    ah_err_t err;

    AH_UNIT_TEST("ah_buf_init() rejects invalid arguments.")
    {
        AH_UNIT_CASE("`b` is NULL.")
        {
            err = ah_buf_init(NULL, buffer, sizeof(buffer));
            AH_UNIT_EQ_ERR(AH_EINVAL, err);
        }

        AH_UNIT_CASE("`base` is NULL and `sz` is positive.")
        {
            err = ah_buf_init(&b, NULL, 1u);
            AH_UNIT_EQ_ERR(AH_EINVAL, err);
        }

#if AH_BUF_SIZE_MAX < SIZE_MAX
        AH_UNIT_CASE("`sz` overflows the size type in ah_buf_t.")
        {
            err ah_buf_init(&b, buffer, AH_BUF_SIZE_MAX + 1u);
            AH_UNIT_EQ_ERR(AH_EOVERFLOW, err);
        }
#endif
    }

    AH_UNIT_TEST("ah_buf_init() accepts unusual but valid arguments.")
    {
        AH_UNIT_CASE("`base` is NULL and `sz` is 0u.")
        {
            err = ah_buf_init(&b, NULL, 0u);
            AH_UNIT_EQ_ERR(AH_OK, err);
        }

        AH_UNIT_CASE("`base` is non-NULL and `sz` is 0u.")
        {
            err = ah_buf_init(&b, buffer, 0u);
            AH_UNIT_EQ_ERR(AH_OK, err);
        }
    }

    AH_UNIT_TEST("ah_buf_init_from_readable() rejects invalid arguments.")
    {
        AH_UNIT_CASE("`b` is NULL.")
        {
            ah_bufc_t c = ah_bufc_from_readable(buffer, sizeof(buffer));

            err = ah_buf_init_from_readable(NULL, &c);
            AH_UNIT_EQ_ERR(AH_EINVAL, err);
        }

        AH_UNIT_CASE("`c` is NULL.")
        {
            err = ah_buf_init_from_readable(&b, NULL);
            AH_UNIT_EQ_ERR(AH_EINVAL, err);
        }
    }

    AH_UNIT_TEST("ah_buf_init_from_readable() refers to readable part of `c`.")
    {
        ah_bufc_t c = ah_bufc_from_readable(buffer, sizeof(buffer));
        ah_bufc_skip(&c, 1u);

        err = ah_buf_init_from_readable(&b, &c);
        if (AH_UNIT_EQ_ERR(AH_OK, err)) {
            AH_UNIT_EQ_UINT(sizeof(buffer) - 1u, ah_buf_get_sz(&b));
        }
    }

    AH_UNIT_TEST("ah_buf_init_from_writable() rejects invalid arguments.")
    {
        AH_UNIT_CASE("`b` is NULL.")
        {
            ah_bufc_t c = ah_bufc_from_writable(buffer, sizeof(buffer));

            err = ah_buf_init_from_writable(NULL, &c);
            AH_UNIT_EQ_ERR(AH_EINVAL, err);
        }

        AH_UNIT_CASE("`c` is NULL.")
        {
            err = ah_buf_init_from_writable(&b, NULL);
            AH_UNIT_EQ_ERR(AH_EINVAL, err);
        }
    }

    AH_UNIT_TEST("ah_buf_init_from_writable() refers to writable part of `c`.")
    {
        ah_bufc_t c = ah_bufc_from_writable(buffer, sizeof(buffer));
        ah_bufc_write_v(&c, 2u);

        err = ah_buf_init_from_writable(&b, &c);
        if (AH_UNIT_EQ_ERR(AH_OK, err)) {
            AH_UNIT_EQ_UINT(sizeof(buffer) - 2u, ah_buf_get_sz(&b));
        }
    }
    AH_UNIT_TEST("ah_buf_get_base() returns NULL if `b` is NULL.")
    {
        AH_UNIT_EQ_PTR(NULL, ah_buf_get_base(NULL));
    }


    AH_UNIT_TEST("ah_buf_get_base() returns unmodified base pointer.")
    {
        err = ah_buf_init(&b, buffer, sizeof(buffer));
        if (AH_UNIT_EQ_ERR(AH_OK, err)) {
            AH_UNIT_EQ_PTR(buffer, ah_buf_get_base(&b));
        }
    }

    AH_UNIT_TEST("ah_buf_get_sz() returns 0u if `b` is NULL.")
    {
        AH_UNIT_EQ_ERR(0u, ah_buf_get_sz(NULL));
    }

    AH_UNIT_TEST("ah_buf_get_sz() returns size in bytes.")
    {
        err = ah_buf_init(&b, buffer, sizeof(buffer));
        if (AH_UNIT_EQ_ERR(AH_OK, err)) {
            AH_UNIT_EQ_UINT(sizeof(buffer), ah_buf_get_sz(&b));
        }
    }

    AH_UNIT_TEST("ah_buf_is_empty() returns true if `b` is NULL.")
    {
        AH_UNIT_EQ_BOOL(true, ah_buf_is_empty(NULL));
    }

    AH_UNIT_TEST("ah_buf_is_empty() returns true if `b` has a NULL base.")
    {
        err = ah_buf_init(&b, NULL, 0u);
        if (AH_UNIT_EQ_ERR(AH_OK, err)) {
            AH_UNIT_EQ_BOOL(true, ah_buf_is_empty(&b));
        }
    }

    AH_UNIT_TEST("ah_buf_is_empty() returns true if `sz` of `b` is 0u.")
    {
        err = ah_buf_init(&b, buffer, 0u);
        if (AH_UNIT_EQ_ERR(AH_OK, err)) {
            AH_UNIT_EQ_BOOL(true, ah_buf_is_empty(&b));
        }
    }

    AH_UNIT_TEST("ah_buf_is_empty() returns false if `b` is not empty.")
    {
        err = ah_buf_init(&b, buffer, sizeof(buffer));
        if (AH_UNIT_EQ_ERR(AH_OK, err)) {
            AH_UNIT_EQ_BOOL(false, ah_buf_is_empty(&b));
        }
    }
}
