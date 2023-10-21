// SPDX-License-Identifier: EPL-2.0

#ifndef AHI_ALLOC_H
#define AHI_ALLOC_H

#include <stddef.h>

#define ahi_bump_alloc_attributes \
    __attribute((alloc_size(2), assume_aligned(sizeof(intptr_t))))

ah_err_t ahi_align_ptr(uintptr_t alignment, uintptr_t* ptr);

#if SIZE_MAX == UINTPTR_MAX
static inline ah_err_t ahi_align_sz(uintptr_t alignment, size_t* sz)
{
    return ahi_align_ptr(alignment, (uintptr_t*) sz);
}
#else
ah_err_t ahi_align_sz(size_t alignment, size_t* sz)
#endif

#endif
