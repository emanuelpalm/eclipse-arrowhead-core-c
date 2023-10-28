// SPDX-License-Identifier: EPL-2.0

#include "ah/alloc.h"

#include <ah/unit.h>
#include <string.h>

typedef struct ahi_thing {
    int32_t n;
    int32_t* sum;
} ahi_thing_t;

static void ahi_on_free(void* ptr);

AH_UNIT_SUITE(alloc_slab)
{
    ah_slab_t s;
    ah_err_t err;

    AH_UNIT_TEST("ah_slab_init() rejects invalid arguments.")
    {
        AH_UNIT_CASE("`s` is NULL.")
        {
            err = ah_slab_init(NULL, 4u);
            AH_UNIT_EQ_ERR(AH_EINVAL, err);
        }

        AH_UNIT_CASE("`slot_sz` being too large causes an overflow.")
        {
            err = ah_slab_init(&s, SIZE_MAX);
            AH_UNIT_EQ_ERR(AH_ERANGE, err);
        }
    }

    AH_UNIT_TEST("ah_slab_init() accepts a `slot_sz` of 0u.")
    {
        err = ah_slab_init(&s, 0u);
        if (AH_UNIT_EQ_ERR(AH_OK, err)) {
            void* a0 = ah_slab_alloc(&s);
            AH_UNIT_NE_PTR(NULL, a0);

            ah_slab_term(&s, NULL);
        }
    }

    AH_UNIT_TEST("ah_slab_alloc() rejects `s` being NULL.")
    {
        void* a0 = ah_slab_alloc(NULL);
        AH_UNIT_EQ_PTR(NULL, a0);
    }

    AH_UNIT_TEST("ah_slab_alloc() returns a writable non-NULL pointer.")
    {
        err = ah_slab_init(&s, 8u);
        if (AH_UNIT_EQ_ERR(AH_OK, err)) {
            void* a0 = ah_slab_alloc(&s);
            AH_UNIT_NE_PTR(NULL, a0);

            strncpy(a0, "abcdefg", 8u);
            AH_UNIT_EQ_STR("abcdefg", a0);

            ah_slab_free(&s, a0);
            ah_slab_term(&s, NULL);
        }
    }

    AH_UNIT_TEST("ah_slab_alloc() returns aligned pointers.")
    {
        err = ah_slab_init(&s, sizeof(intptr_t));
        if (AH_UNIT_EQ_ERR(AH_OK, err)) {
            uint8_t* a0 = ah_slab_alloc(&s);
            uint8_t* a1 = ah_slab_alloc(&s);
            uint8_t* a2 = ah_slab_alloc(&s);
            uint8_t* a3 = ah_slab_alloc(&s);

            AH_UNIT_EQ_UHEX(0u, ((uintptr_t) a0) & (sizeof(intptr_t) - 1u));
            AH_UNIT_EQ_UHEX(0u, ((uintptr_t) a1) & (sizeof(intptr_t) - 1u));
            AH_UNIT_EQ_UHEX(0u, ((uintptr_t) a2) & (sizeof(intptr_t) - 1u));
            AH_UNIT_EQ_UHEX(0u, ((uintptr_t) a3) & (sizeof(intptr_t) - 1u));
        }
    }

    AH_UNIT_TEST("ah_slab_free() silently rejects invalid arguments.")
    {
        AH_UNIT_CASE("`s` is NULL.")
        {
            ah_slab_free(NULL, NULL);
            // Nothing to check.
        }

        AH_UNIT_CASE("`ptr` is NULL.")
        {
            err = ah_slab_init(&s, 4u);
            if (AH_UNIT_EQ_ERR(AH_OK, err)) {
                ah_slab_free(&s, NULL);
                // Nothing to check.
            }
        }
    }

    AH_UNIT_TEST("ah_slab_term() silently rejects `s` being NULL.")
    {
        ah_slab_term(NULL, NULL);
        // Nothing to check.
    }

    AH_UNIT_TEST("ah_slab_term() calls allocated slots.")
    {
        err = ah_slab_init(&s, sizeof(ahi_thing_t));
        if (AH_UNIT_EQ_ERR(AH_OK, err)) {
            ahi_thing_t* a0 = ah_slab_alloc(&s);
            ahi_thing_t* a1 = ah_slab_alloc(&s);
            ahi_thing_t* a2 = ah_slab_alloc(&s);

            int32_t sum = 0u;
            *a0 = (ahi_thing_t) { .n = 100, .sum = &sum };
            *a1 = (ahi_thing_t) { .n = 20, .sum = &sum };
            *a2 = (ahi_thing_t) { .n = 3, .sum = &sum };

            ah_slab_free(&s, a1);

            ah_slab_term(&s, ahi_on_free);
            AH_UNIT_EQ_UINT(103, sum);
        }
    }
}

static void ahi_on_free(void* ptr)
{
    ahi_thing_t* thing = ptr;
    *thing->sum += thing->n;
}
