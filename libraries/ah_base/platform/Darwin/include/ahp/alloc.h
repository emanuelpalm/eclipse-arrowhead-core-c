// SPDX-License-Identifier: EPL-2.0

#ifndef AHP_ALLOC_H
#define AHP_ALLOC_H

#include <stddef.h>

void* ahp_page_alloc(size_t sz);
void ahp_page_free(void* ptr, size_t sz);
size_t ahp_page_get_size(void);

#endif
