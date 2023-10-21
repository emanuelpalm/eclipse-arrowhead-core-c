// SPDX-License-Identifier: EPL-2.0

#include "ah/alloc.h"

#include <ah/unit.h>

AH_UNIT_SUITE(alloc_bump)
{
    intptr_t region[4u] = { 0u };
    ah_bump_t b;
    ah_err_t err;

    AH_UNIT_TEST("ah_bump_init() rejects invalid arguments.")
    {
        AH_UNIT_CASE("`b` is NULL.")
        {
            err = ah_bump_init(NULL, &region, sizeof(region));
            AH_UNIT_EQ_ERR(AH_EINVAL, err);
        }

        AH_UNIT_CASE("`base` is NULL while `sz` is positive.")
        {
            err = ah_bump_init(&b, NULL, sizeof(region));
            AH_UNIT_EQ_ERR(AH_EINVAL, err);
        }

        AH_UNIT_CASE("Aligning `base` overflows uintptr_t.")
        {
            err = ah_bump_init(&b, (void*) UINTPTR_MAX, sizeof(region));
            AH_UNIT_EQ_ERR(ERANGE, err);
        }

        AH_UNIT_CASE("Adding aligned `base` and `sz` overflows uintptr_t.")
        {
            err = ah_bump_init(&b, &region, UINTPTR_MAX);
            AH_UNIT_EQ_ERR(ERANGE, err);
        }
    }

    AH_UNIT_TEST("ah_bump_init() accepts unusual but valid arguments.")
    {
        AH_UNIT_CASE("`base` is NULL and `sz` is 0u.")
        {
            err = ah_bump_init(&b, NULL, 0u);
            if (AH_UNIT_EQ_ERR(AH_OK, err)) {
                void* a0 = ah_bump_alloc(&b, 1u);
                AH_UNIT_EQ_PTR(NULL, a0);
            }
        }

        AH_UNIT_CASE("`base` is non-NULL and `sz` is 0u.")
        {
            err = ah_bump_init(&b, &region, 0u);
            if (AH_UNIT_EQ_ERR(AH_OK, err)) {
                void* a0 = ah_bump_alloc(&b, 1u);
                AH_UNIT_EQ_PTR(NULL, a0);
            }
        }
    }

    AH_UNIT_TEST("ah_bump_alloc() rejects invalid arguments.")
    {
        AH_UNIT_CASE("`b` is NULL.")
        {
            void* a0 = ah_bump_alloc(NULL, 4u);
            AH_UNIT_EQ_PTR(NULL, a0);
        }

        AH_UNIT_CASE("Aligning `sz` overflows uintptr_t.")
        {
            err = ah_bump_init(&b, (void*) 1u, UINTPTR_MAX - sizeof(intptr_t));
            if (AH_UNIT_EQ_ERR(AH_OK, err)) {
                void* a0 = ah_bump_alloc(&b, UINTPTR_MAX);
                void* a1 = ah_bump_alloc(&b, UINTPTR_MAX - sizeof(intptr_t));

                AH_UNIT_EQ_PTR(NULL, a0);
                AH_UNIT_EQ_PTR(NULL, a1);
            }
        }
    }

    AH_UNIT_TEST("ah_bump_alloc() returns aligned pointers.")
    {
#if UINTPTR_MAX >= UINT32_MAX
        err = ah_bump_init(&b, &region, sizeof(region));
        if (AH_UNIT_EQ_ERR(AH_OK, err)) {
            void* a0 = ah_bump_alloc(&b, 1u);
            void* a1 = ah_bump_alloc(&b, sizeof(intptr_t) + 2u);
            void* a2 = ah_bump_alloc(&b, 3u);
            void* a3 = ah_bump_alloc(&b, 1u);

            AH_UNIT_EQ_PTR(&region[0u], a0);
            AH_UNIT_EQ_PTR(&region[1u], a1);
            AH_UNIT_EQ_PTR(&region[3u], a2);
            AH_UNIT_EQ_PTR(NULL, a3);
        }
#else
        AH_UNIT_SKIP("Requires platform pointer size of at least 4.");
#endif
    }

    AH_UNIT_TEST("ah_bump_reset() resets bump allocator.")
    {
        err = ah_bump_init(&b, &region, sizeof(region));
        if (AH_UNIT_EQ_ERR(AH_OK, err)) {
            void* a0 = ah_bump_alloc(&b, 1u);
            void* a1 = ah_bump_alloc(&b, 2u);

            ah_bump_reset(&b);

            void* a2 = ah_bump_alloc(&b, 3u);

            AH_UNIT_GE_UHEX((uintptr_t) a1, ((uintptr_t) a0) + 1u);
            AH_UNIT_EQ_PTR(a0, a2);
        }
    }

    AH_UNIT_TEST("ah_bump_get_capacity() reports correct capacity.")
    {
        err = ah_bump_init(&b, &region, sizeof(region));
        if (AH_UNIT_EQ_ERR(AH_OK, err)) {
            size_t c0 = ah_bump_get_capacity(&b);

            (void) ah_bump_alloc(&b, 1u);
            (void) ah_bump_alloc(&b, 2u);
            (void) ah_bump_alloc(&b, 3u);

            size_t c1 = ah_bump_get_capacity(&b);

            AH_UNIT_EQ_UINT(sizeof(region), c0);
            AH_UNIT_EQ_UINT(sizeof(region), c1);
        }
    }

    AH_UNIT_TEST("ah_bump_get_used_size() reports correct size.")
    {
        ah_bump_init(&b, &region, sizeof(region));
        if (AH_UNIT_EQ_ERR(AH_OK, err)) {
            size_t u0 = ah_bump_get_used_size(&b);
            (void) ah_bump_alloc(&b, 1u);
            size_t u1 = ah_bump_get_used_size(&b);
            (void) ah_bump_alloc(&b, 1u);
            size_t u2 = ah_bump_get_used_size(&b);
            (void) ah_bump_alloc(&b, 1u);
            size_t u3 = ah_bump_get_used_size(&b);

            AH_UNIT_EQ_UINT(sizeof(intptr_t) * 0u, u0);
            AH_UNIT_EQ_UINT(sizeof(intptr_t) * 1u, u1);
            AH_UNIT_EQ_UINT(sizeof(intptr_t) * 2u, u2);
            AH_UNIT_EQ_UINT(sizeof(intptr_t) * 3u, u3);
        }
    }

    AH_UNIT_TEST("ah_bump_get_free_size() reports correct size.")
    {
        ah_bump_init(&b, &region, sizeof(region));
        if (AH_UNIT_EQ_ERR(AH_OK, err)) {
            size_t f0 = ah_bump_get_free_size(&b);
            (void) ah_bump_alloc(&b, 1u);
            size_t f1 = ah_bump_get_free_size(&b);
            (void) ah_bump_alloc(&b, 1u);
            size_t f2 = ah_bump_get_free_size(&b);
            (void) ah_bump_alloc(&b, 1u);
            size_t f3 = ah_bump_get_free_size(&b);

            AH_UNIT_EQ_UINT(sizeof(intptr_t) * 4u, f0);
            AH_UNIT_EQ_UINT(sizeof(intptr_t) * 3u, f1);
            AH_UNIT_EQ_UINT(sizeof(intptr_t) * 2u, f2);
            AH_UNIT_EQ_UINT(sizeof(intptr_t) * 1u, f3);
        }
    }
}
