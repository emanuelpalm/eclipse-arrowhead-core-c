// SPDX-License-Identifier: EPL-2.0

#include "ah/bit.h"
#include "ah/buf.h"

#include <ah/unit.h>

AH_UNIT_SUITE(bufc_skip)
{
    AH_UNIT_TEST("ah_bufc_skip() rejects invalid arguments.")
    {
        AH_UNIT_CASE("`c` is NULL.")
        {
            AH_UNIT_EQ_BOOL(false, ah_bufc_skip(NULL, 1));
        }

        AH_UNIT_CASE("`n` is too large for `c`.")
        {
            ah_bufc_t c = ah_bufc_from_readable(NULL, 0u);
            AH_UNIT_EQ_BOOL(false, ah_bufc_skip(&c, 1u));
        }
    }

    AH_UNIT_TEST("ah_bufc_skip() successfully skips bytes.")
    {
        uint8_t buffer[32u];

        ah_bufc_t c = ah_bufc_from_readable(buffer, sizeof(buffer));

        AH_UNIT_EQ_BOOL(true, ah_bufc_skip(&c, 0u));
        AH_UNIT_EQ_UINT(32u, ah_bufc_get_readable_sz(&c));

        AH_UNIT_EQ_BOOL(true, ah_bufc_skip(&c, 1u));
        AH_UNIT_EQ_UINT(31u, ah_bufc_get_readable_sz(&c));

        AH_UNIT_EQ_BOOL(true, ah_bufc_skip(&c, 14u));
        AH_UNIT_EQ_UINT(17u, ah_bufc_get_readable_sz(&c));

        AH_UNIT_EQ_BOOL(true, ah_bufc_skip(&c, 0u));
        AH_UNIT_EQ_UINT(17u, ah_bufc_get_readable_sz(&c));

        AH_UNIT_EQ_BOOL(true, ah_bufc_skip(&c, 8u));
        AH_UNIT_EQ_UINT(9u, ah_bufc_get_readable_sz(&c));

        AH_UNIT_EQ_BOOL(true, ah_bufc_skip(&c, 9u));
        AH_UNIT_EQ_UINT(0u, ah_bufc_get_readable_sz(&c));

        AH_UNIT_EQ_BOOL(false, ah_bufc_skip(&c, 1u));
    }

    AH_UNIT_TEST("ah_bufc_skip_all() does nothing if `c` is NULL.")
    {
        ah_bufc_skip_all(NULL);
    }

    AH_UNIT_TEST("ah_bufc_skip_all() skips all readable bytes in `c`.")
    {
        uint8_t buffer[32u];
        ah_bufc_t c = ah_bufc_from_readable(buffer, sizeof(buffer));
        AH_UNIT_EQ_UINT(32u, ah_bufc_get_readable_sz(&c));

        ah_bufc_skip_all(&c);
        AH_UNIT_EQ_UINT(0u, ah_bufc_get_readable_sz(&c));

        ah_bufc_skip_all(&c);
        AH_UNIT_EQ_UINT(0u, ah_bufc_get_readable_sz(&c));
    }
}
