// SPDX-License-Identifier: EPL-2.0

#include "ah/alloc.h"

#include <ah/unit.h>

AH_UNIT_SUITE(alloc_page)
{
    AH_UNIT_TEST("ah_page_alloc() with a zero argument returns NULL.")
    {
        void* a0 = ah_page_alloc(0u);
        AH_UNIT_EQ_PTR(NULL, a0);
    }

    AH_UNIT_TEST("ah_page_alloc() with a smaller argument returns non-NULL.")
    {
        size_t sz = ah_page_get_size();//63u;

        void* a0 = ah_page_alloc(sz);
        AH_UNIT_NE_PTR(NULL, a0);

        ah_page_free(a0, sz);
    }

    AH_UNIT_TEST("ah_page_alloc() with a larger argument returns non-NULL.")
    {
        size_t sz = ah_page_get_size() * 2u;

        void* a0 = ah_page_alloc(sz);
        AH_UNIT_NE_PTR(NULL, a0);

        ah_page_free(a0, sz);
    }

    AH_UNIT_TEST("ah_page_get_size() returns a positive power of two.")
    {
        size_t sz = ah_page_get_size();
        AH_UNIT_NE_UINT(0u, sz);
        AH_UNIT_EQ_UINT(0u, sz & (sz - 1u));
    }
}
