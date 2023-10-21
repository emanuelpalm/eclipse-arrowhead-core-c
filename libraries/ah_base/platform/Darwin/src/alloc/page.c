// SPDX-License-Identifier: EPL-2.0

#include "ahp/err.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define AHI_MMAP_PROT  (PROT_READ | PROT_WRITE)
#define AHI_MMAP_FLAGS (MAP_ANONYMOUS | MAP_PRIVATE)

void* ahp_page_alloc(size_t sz)
{
    if (sz == 0u) {
        return NULL;
    }

    void* ptr = mmap(NULL, sz, AHI_MMAP_PROT, AHI_MMAP_FLAGS, -1, 0);
    if (ptr != MAP_FAILED) {
        return ptr;
    }

    if (errno == ENOMEM) {
        return NULL;
    }

    (void) fprintf(stderr,
        "mmap(NULL, %zu, AHI_MMAP_PROT, AHI_MMAP_FLAGS, -1, 0) failed; %s.",
        sz, ahp_err_get_s(errno));

    abort();
}

void ahp_page_free(void* ptr, size_t sz)
{
    if (munmap(ptr, sz) != -1) {
        return;
    }

    (void) fprintf(stderr,
        "munmap(%p, %zu) failed; %s.",
        ptr, sz, ahp_err_get_s(errno));

    abort();
}

size_t ahp_page_get_size(void)
{
    static size_t cached_page_size = 0u;

    size_t page_size = __atomic_load_n(&cached_page_size, __ATOMIC_RELAXED);
    if (page_size != 0u) {
        return page_size;
    }

    long value = sysconf(_SC_PAGE_SIZE);
    if (value != -1) {
        page_size = value;
        __atomic_store_n(&cached_page_size, page_size, __ATOMIC_RELAXED);
        return page_size;
    }

    (void) fprintf(stderr,
        "sysconf(_SC_PAGE_SIZE) failed; %s.",
        ahp_err_get_s(errno));

    abort();
}
