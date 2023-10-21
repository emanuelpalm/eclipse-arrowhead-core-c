// SPDX-License-Identifier: EPL-2.0

#include "ah/alloc.h"
#include "ah/ckdint.h"
#include "ah/err.h"

ah_err_t ahi_align_ptr(uintptr_t alignment, uintptr_t* ptr)
{
    if (ptr == NULL) {
        return AH_EINVAL;
    }

    if (alignment == 0u || (alignment & (alignment - 1u)) != 0u) {
        return AH_EDOM;
    }

    if (ah_ckd_add(ptr, *ptr, alignment - 1u)) {
        return AH_ERANGE;
    }
    *ptr &= ~(alignment - 1u);

    return AH_OK;
}

#if SIZE_MAX != UINTPTR_MAX
ah_err_t ahi_align_sz(size_t alignment, size_t* sz)
{
    if (sz == NULL) {
        return AH_EINVAL;
    }

    if (alignment == 0u || (alignment & (alignment - 1u)) != 0u) {
        return AH_EDOM;
    }

    if (ah_ckd_add(sz, *sz, alignment - 1u)) {
        return AH_ERANGE;
    }
    *sz &= ~(alignment - 1u);

    return AH_OK;
}
#endif
