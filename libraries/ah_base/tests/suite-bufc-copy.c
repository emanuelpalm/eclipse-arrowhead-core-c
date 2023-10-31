// SPDX-License-Identifier: EPL-2.0

#include "ah/buf.h"

#include <ah/unit.h>

AH_UNIT_SUITE(bufc_copy)
{
    AH_UNIT_TEST("ah_bufc_copy() rejects invalid arguments.")
    {
        uint8_t mem[32u] = { 0u };

        AH_UNIT_CASE("`src` is NULL.")
        {
            ah_bufc_t dst = ah_bufc_from_writable(mem, sizeof(mem));
            AH_UNIT_EQ_BOOL(false, ah_bufc_copy(NULL, &dst, 1u));
        }

        AH_UNIT_CASE("`dst` is NULL.")
        {
            ah_bufc_t src = ah_bufc_from_readable(mem, sizeof(mem));
            AH_UNIT_EQ_BOOL(false, ah_bufc_copy(&src, NULL, 1u));
        }

        AH_UNIT_CASE("`n` is too large for `src`.")
        {
            ah_bufc_t src = ah_bufc_from_readable(NULL, 0u);
            ah_bufc_t dst = ah_bufc_from_writable(mem, sizeof(mem));
            AH_UNIT_EQ_BOOL(false, ah_bufc_copy(&src, &dst, 1u));
        }

        AH_UNIT_CASE("`n` is too large for `dst`.")
        {
            ah_bufc_t src = ah_bufc_from_readable(mem, sizeof(mem));
            ah_bufc_t dst = ah_bufc_from_writable(NULL, 0u);
            AH_UNIT_EQ_BOOL(false, ah_bufc_copy(&src, &dst, 1u));
        }
    }

    AH_UNIT_TEST("ah_bufc_copy() copies from `src` to `dst`.")
    {
        const char* src_mem = "A string of text.";
        ah_bufc_t src = ah_bufc_from_readable(src_mem, strlen(src_mem));

        char dst_mem[24u];
        ah_bufc_t dst = ah_bufc_from_writable(dst_mem, sizeof(dst_mem));

        // Copy the source buffer in parts.
        AH_UNIT_EQ_BOOL(true, ah_bufc_copy(&src, &dst, 8u));
        AH_UNIT_EQ_BOOL(true, ah_bufc_copy(&src, &dst, 3u));
        AH_UNIT_EQ_BOOL(true, ah_bufc_copy(&src, &dst, 6u));

        // There is nothing left to read, but space left for writing.
        AH_UNIT_EQ_BOOL(false, ah_bufc_copy(&src, &dst, 1u));

        // Terminate the written string.
        AH_UNIT_EQ_BOOL(true, ah_bufc_write_u8(&dst, '\0'));

        AH_UNIT_EQ_STR(src_mem, dst_mem);
    }
}
