// SPDX-License-Identifier: EPL-2.0

#ifndef AHP_ALLOC_H
#define AHP_ALLOC_H

#include <stddef.h>

#define ahp_page_alloc_attrs                                                   \
    __attribute((alloc_size(1), assume_aligned(sizeof(intptr_t))))

ahp_page_alloc_attrs void* ahp_page_alloc(size_t sz);

void ahp_page_free(void* ptr, size_t sz);
size_t ahp_page_get_size(void);

#endif
