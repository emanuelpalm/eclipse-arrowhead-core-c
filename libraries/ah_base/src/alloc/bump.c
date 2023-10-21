// SPDX-License-Identifier: EPL-2.0

#include "ah/alloc.h"
#include "ah/ckdint.h"
#include "ah/err.h"

ah_err_t ah_bump_init(ah_bump_t* b, void* base, size_t sz)
{
    if (b == NULL || (base == NULL && sz != 0u)) {
        return AH_EINVAL;
    }

    if (ah_align_ptr(sizeof(intptr_t), (uintptr_t*) &base) != AH_OK) {
        return AH_ERANGE;
    }

    uint8_t* end;
    if (ah_ckd_add((uintptr_t*) &end, (uintptr_t) base, sz)) {
        return AH_ERANGE;
    }

    *b = (ah_bump_t) {
        .base = base,
        .off = base,
        .end = end,
    };

    return AH_OK;
}

void* ah_bump_alloc(ah_bump_t* b, size_t sz)
{
    if (b == NULL) {
        return NULL;
    }

    uint8_t* ptr = b->off;
    uint8_t* off;
    if (ah_ckd_add((uintptr_t*) &off, (uintptr_t) ptr, sz)) {
        return NULL;
    }

    if (ah_align_ptr(sizeof(intptr_t), (uintptr_t*) &off) != AH_OK) {
        return NULL;
    }

    if (off > b->end) {
        return NULL;
    }

    b->off = off;

    return ptr;
}
