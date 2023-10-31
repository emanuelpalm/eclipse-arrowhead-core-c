// SPDX-License-Identifier: EPL-2.0

#include "ah/buf.h"

#include <ah/unit.h>

AH_UNIT_SUITE(bufc_peek)
{
    uint8_t buffer[32u] = { 0u };
    ah_bufc_t c;

    AH_UNIT_TEST("ah_bufc_peek() rejects invalid arguments.")
    {
        AH_UNIT_CASE("`c` is NULL.")
        {
            AH_UNIT_EQ_BOOL(false, ah_bufc_peek(NULL, buffer, sizeof(buffer)));
        }

        AH_UNIT_CASE("`dst` is NULL.")
        {
            ah_bufc_t src = ah_bufc_from_readable(buffer, sizeof(buffer));
            AH_UNIT_EQ_BOOL(false, ah_bufc_peek(&src, NULL, 1u));
        }

        AH_UNIT_CASE("`n` is too large for `c`.")
        {
            ah_bufc_t src = ah_bufc_from_readable(NULL, 0u);
            AH_UNIT_EQ_BOOL(false, ah_bufc_peek(&src, buffer, 1u));
        }
    }

    AH_UNIT_TEST("ah_bufc_peek() copies from `c` to `dst`.")
    {
        const char* src_buffer = "A string of text.";
        size_t src_sz = strlen(src_buffer) + 1u;
        c = ah_bufc_from_readable(src_buffer, src_sz);

        AH_UNIT_EQ_BOOL(true, ah_bufc_peek(&c, buffer, src_sz));
        AH_UNIT_EQ_UINT(src_sz, ah_bufc_get_readable_sz(&c));
        AH_UNIT_EQ_STR(src_buffer, (char*) buffer);
    }
}
