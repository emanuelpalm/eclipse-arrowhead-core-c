// SPDX-License-Identifier: EPL-2.0

#include "ah/alloc.h"
#include "ah/ckdint.h"
#include "ah/err.h"

#include <assert.h>

#define AHI_IS_ALLOCATED ((ahi_slab_slot_t*) (-1))

typedef struct ahi_slab_slot ahi_slab_slot_t;
typedef struct ahi_slab_bank ahi_slab_bank_t;

typedef void (*ahi_slab_cb_t)(void*);

struct ahi_slab_slot {
    // Set to AHI_IS_ALLOCATED while this slot is allocated.
    ahi_slab_slot_t* _next_free;

    uint8_t _body[];
};

struct ahi_slab_bank {
    ahi_slab_bank_t* _next;
    uint8_t _body[];
};

static void ahi_call_used(ah_slab_t* s, ahi_slab_bank_t* b, ahi_slab_cb_t cb);

ah_err_t ah_slab_init(ah_slab_t* s, size_t slot_sz)
{
    if (s == NULL) {
        return AH_EINVAL;
    }

    ah_err_t err;

    if (ah_ckd_add(&slot_sz, slot_sz, sizeof(struct ahi_slab_slot))) {
        return AH_ERANGE;
    }
    err = ah_align_sz(sizeof(intptr_t), &slot_sz);
    if (err != AH_OK) {
        return err;
    }

    size_t bank_sz;
    if (ah_ckd_mul(&bank_sz, 4u, slot_sz)) {
        return AH_ERANGE;
    }
    if (ah_ckd_add(&bank_sz, bank_sz, sizeof(ahi_slab_bank_t))) {
        return AH_ERANGE;
    }
    err = ah_align_sz(ah_page_get_size(), &bank_sz);
    if (err != AH_OK) {
        return err;
    }

    size_t slots_per_bank = (bank_sz - sizeof(ahi_slab_bank_t)) / slot_sz;

    *s = (ah_slab_t) {
        ._bank_list = NULL,
        ._bank_sz = bank_sz,
        ._slot_free_list = NULL,
        ._slot_sz = slot_sz,
        ._slots_per_bank = slots_per_bank,
        ._ref_count = 1u,
    };

    return AH_OK;
}

void ah_slab_term(ah_slab_t* s, void (*slot_cb_or_null)(void*))
{
    if (s == NULL || s->_ref_count == 0u) {
        return;
    }

    s->_ref_count -= 1u;
    if (s->_ref_count != 0u && slot_cb_or_null == NULL) {
        return;
    }

    for (ahi_slab_bank_t* this = s->_bank_list; this != NULL;) {
        ahi_slab_bank_t* next = this->_next;

        if (slot_cb_or_null != NULL) {
            ahi_call_used(s, this, slot_cb_or_null);
        }

        ah_page_free(this, s->_bank_sz);
        this = next;
    }
}

static void ahi_call_used(ah_slab_t* s, ahi_slab_bank_t* b, ahi_slab_cb_t cb)
{
    for (size_t i = 0; i < s->_slots_per_bank; i++) {
        ahi_slab_slot_t* slot = (void*) &b->_body[i * s->_slot_sz];
        if (slot->_next_free == AHI_IS_ALLOCATED) {
            cb(slot->_body);
        }
    }
}

void* ah_slab_alloc(ah_slab_t* s)
{
    if (s == NULL || s->_ref_count == 0u) {
        return NULL;
    }

    ahi_slab_slot_t* free_slot = s->_slot_free_list;
    if (free_slot == NULL) {
        ahi_slab_bank_t* bank = ah_page_alloc(s->_bank_sz);
        if (bank == NULL) {
            return NULL;
        }

        bank->_next = s->_bank_list;
        s->_bank_list = bank;

        ahi_slab_slot_t* this_slot = (void*) bank->_body;
        ahi_slab_slot_t* next_slot;
        for (size_t i = 1u; i < s->_slots_per_bank; i++) {
            next_slot = (void*) &bank->_body[i * s->_slot_sz];
            this_slot->_next_free = next_slot;
            this_slot = next_slot;
        }
        next_slot->_next_free = NULL;

        free_slot = (void*) bank->_body;
    }

    s->_ref_count += 1u;
    s->_slot_free_list = free_slot->_next_free;
    free_slot->_next_free = AHI_IS_ALLOCATED;

    return free_slot->_body;
}

void ah_slab_free(ah_slab_t* s, void* ptr)
{
    if (s == NULL || ptr == NULL) {
        return;
    }

    ahi_slab_slot_t* slot = (void*) &((uint8_t*) ptr)[-sizeof(ahi_slab_slot_t)];
    assert(slot->_next_free == AHI_IS_ALLOCATED);
    slot->_next_free = s->_slot_free_list;
    s->_slot_free_list = slot;

    s->_ref_count -= 1u;
    if (s->_ref_count != 0u) {
        return;
    }

    for (ahi_slab_bank_t* this = s->_bank_list; this != NULL;) {
        ahi_slab_bank_t* next = this->_next;
        ah_page_free(this, s->_bank_sz);
        this = next;
    }
}
