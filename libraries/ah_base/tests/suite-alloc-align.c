// SPDX-License-Identifier: EPL-2.0

#include "ah/alloc.h"

#include <ah/unit.h>

AH_UNIT_SUITE(alloc_align)
{
    AH_UNIT_TEST("ah_align_ptr() aligns pointer as expected.")
    {
        struct ah_Case {
            size_t alignment;
            uintptr_t before;
            uintptr_t after;
            ah_err_t err;
        };

        static const struct ah_Case cases[] = {
            { 0u, 0u, 0u, AH_EDOM },

            { 4u, 0u, 0u, AH_OK },
            { 4u, 1u, 4u, AH_OK },
            { 4u, 2u, 4u, AH_OK },
            { 4u, 3u, 4u, AH_OK },
            { 4u, 4u, 4u, AH_OK },
            { 4u, 5u, 8u, AH_OK },
            { 4u, 6u, 8u, AH_OK },
            { 4u, 7u, 8u, AH_OK },
            { 4u, 8u, 8u, AH_OK },
            { 4u, 9u, 12u, AH_OK },

            { 8u, 0u, 0u, AH_OK },
            { 8u, 1u, 8u, AH_OK },
            { 8u, 2u, 8u, AH_OK },
            { 8u, 3u, 8u, AH_OK },
            { 8u, 4u, 8u, AH_OK },
            { 8u, 5u, 8u, AH_OK },
            { 8u, 6u, 8u, AH_OK },
            { 8u, 7u, 8u, AH_OK },
            { 8u, 8u, 8u, AH_OK },
            { 8u, 9u, 16u, AH_OK },

            { 16u, 15u, 16u, AH_OK },
            { 16u, 16u, 16u, AH_OK },
            { 16u, 17u, 32u, AH_OK },

            { 32u, 31u, 32u, AH_OK },
            { 32u, 32u, 32u, AH_OK },
            { 32u, 33u, 64u, AH_OK },

            { 4096u, UINTPTR_MAX, 0u, AH_ERANGE },
        };

        for (size_t i = 0u; i < sizeof(cases) / sizeof(struct ah_Case); i++) {
            struct ah_Case c = cases[i];

            AH_UNIT_CASE("alignment: %zu, before: %p, after: %p, err: %s",
                c.alignment, c.before, c.after, ah_err_get_s(c.err))
            {
                uintptr_t ptr = c.before;
                ah_err_t err = ah_align_ptr(c.alignment, &ptr);
                AH_UNIT_EQ_ERR(c.err, err);
                if (c.err == AH_OK) {
                    AH_UNIT_EQ_UHEX(c.after, (uintptr_t) ptr);
                }
            }
        }
    }

    AH_UNIT_TEST("ah_align_ptr() returns AH_EINVAL if ptr is NULL.")
    {
        ah_err_t err = ah_align_ptr(0u, NULL);
        AH_UNIT_EQ_ERR(AH_EINVAL, err);
    }

    AH_UNIT_TEST("ah_align_sz() aligns size as expected.")
    {
        struct ah_Case {
            size_t alignment;
            size_t before;
            size_t after;
            ah_err_t err;
        };

        static const struct ah_Case cases[] = {
            { 0u, 0u, 0u, AH_EDOM },

            { 4u, 0u, 0u, AH_OK },
            { 4u, 1u, 4u, AH_OK },
            { 4u, 2u, 4u, AH_OK },
            { 4u, 3u, 4u, AH_OK },
            { 4u, 4u, 4u, AH_OK },
            { 4u, 5u, 8u, AH_OK },
            { 4u, 6u, 8u, AH_OK },
            { 4u, 7u, 8u, AH_OK },
            { 4u, 8u, 8u, AH_OK },
            { 4u, 9u, 12u, AH_OK },

            { 8u, 0u, 0u, AH_OK },
            { 8u, 1u, 8u, AH_OK },
            { 8u, 2u, 8u, AH_OK },
            { 8u, 3u, 8u, AH_OK },
            { 8u, 4u, 8u, AH_OK },
            { 8u, 5u, 8u, AH_OK },
            { 8u, 6u, 8u, AH_OK },
            { 8u, 7u, 8u, AH_OK },
            { 8u, 8u, 8u, AH_OK },
            { 8u, 9u, 16u, AH_OK },

            { 16u, 15u, 16u, AH_OK },
            { 16u, 16u, 16u, AH_OK },
            { 16u, 17u, 32u, AH_OK },

            { 32u, 31u, 32u, AH_OK },
            { 32u, 32u, 32u, AH_OK },
            { 32u, 33u, 64u, AH_OK },

            { 4096u, SIZE_MAX, 0u, AH_ERANGE },
        };

        for (size_t i = 0u; i < sizeof(cases) / sizeof(struct ah_Case); i++) {
            struct ah_Case c = cases[i];

            AH_UNIT_CASE("alignment: %zu, before: %p, after: %p, err: %s",
                c.alignment, c.before, c.after, ah_err_get_s(c.err))
            {
                size_t sz = c.before;
                ah_err_t err = ah_align_sz(c.alignment, &sz);
                AH_UNIT_EQ_ERR(c.err, err);
                if (c.err == AH_OK) {
                    AH_UNIT_EQ_UHEX(c.after, sz);
                }
            }
        }
    }

    AH_UNIT_TEST("ah_align_sz() returns AH_EINVAL if sz is NULL.")
    {
        ah_err_t err = ah_align_sz(0u, NULL);
        AH_UNIT_EQ_ERR(AH_EINVAL, err);
    }
}
