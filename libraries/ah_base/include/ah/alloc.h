// SPDX-License-Identifier: EPL-2.0

#ifndef AH_ALLOC_H
#define AH_ALLOC_H

/**
 * @file
 * Heap memory allocators.
 *
 * Functions and types for allocating and deallocating heap memory.
 */

#include "def.h"
#include "err.h"

#include <ahi/alloc.h>
#include <ahp/alloc.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Bump allocator.
 *
 * Portions out variably-sized subregions of a fixed memory region owned by the
 * allocator. These subregions cannot be freed individually. They can, however,
 * be freed all at once by resetting the allocator.
 */
struct ah_bump {
    uint8_t* base; ///< Pointer to beginning of memory region.
    uint8_t* off;  ///< Pointer to first unused byte in memory region.
    uint8_t* end;  ///< Pointer to first byte after end of memory region.
};

/**
 * Slab allocator.
 *
 * Maintains a variable number of memory <em>slots</em> that can be
 * individually allocated and freed, each of which has the same size in bytes.
 * These slots are stored in <em>banks</em>, which, in turn, are stored in
 * memory pages allocated via ah_page_alloc().
 */
struct ah_slab {
    struct ahi_slab_bank* _bank_list;
    size_t _bank_sz;

    struct ahi_slab_slot* _slot_free_list;
    size_t _slot_sz;
    size_t _slots_per_bank;

    size_t _ref_count;
};

/**
 * @name Memory Alignment
 *
 * Functions for aligning pointers and sizes to significant memory boundaries,
 * all of which must be multiples of the platform pointer size,
 * @c sizeof(intptr_t).
 *
 * @{
 */

/**
 * Rounds up referenced pointer @a ptr to its nearest multiple of @a alignment.
 *
 * @param[in]     alignment Integer multiple of the platform pointer size.
 * @param[in,out] ptr       Pointer value to round up to nearest multiple of
 *                          @a alignment.
 *
 * @retval AH_OK     if successful.
 * @retval AH_EDOM   if @a alignment is not a positive power of 2.
 * @retval AH_EINVAL if @a ptr is @c NULL.
 * @retval AH_ERANGE if the operation overflowed @a ptr.
 *
 * @warning If anything but @c AH_OK is returned, the state of the pointer
 *          referred to by @a ptr is undefined.
 *
 * @warning This operation may increase the address of the pointer referred to
 *          by @a ptr with up to @c alignment-1. It is your responsibility to
 *          ensure this address is valid.
 */
ah_inline ah_err_t ah_align_ptr(uintptr_t alignment, uintptr_t* ptr)
{
    return ahi_align_ptr(alignment, ptr);
}

/**
 * Rounds up referenced size @a sz its nearest multiple of @a alignment.
 *
 * @param[in]     alignment Integer multiple of the platform pointer size.
 * @param[in,out] sz        Size to round up nearest multiple of @a alignment.
 *
 * @retval AH_OK     if successful.
 * @retval AH_EDOM   if @a alignment is not a positive power of 2.
 * @retval AH_EINVAL if @a sz is @c NULL.
 * @retval AH_ERANGE if the operation overflowed @a sz.
 *
* @warning If anything but @c AH_OK is returned, the state of the size referred
*          to by @a sz is undefined.
 */
ah_inline ah_err_t ah_align_sz(uintptr_t alignment, size_t* sz)
{
    return ahi_align_sz(alignment, sz);
}

/** @} */

/**
 * @name Bump Allocation
 *
 * A bump allocator maintains a fixed-size region of memory from which
 * variably-sized subregions can be allocated.
 *
 * Every time a subregion is allocated, the allocator increments, or @a bumps,
 * a pointer to the first byte right after the allocated subregion. Since no
 * record is kept of how large the individual subregions are, they cannot be
 * freed individually. Rather, the bump allocator is freed by @a resetting it,
 * which means that the mentioned pointer is updated to point at the first byte
 * in its fixed-size memory region.
 *
 * It is your responsibility to ensure that any destructuring routines relevant
 * to the allocated subregions are executed before the allocator is reset or
 * its memory is freed.
 *
 * @{
 */

/**
 * Initializes bump allocator for subsequent use.
 *
 * @param[out] b    Pointer to bump allocator to initialize.
 * @param[in]  base Pointer to memory region from which to allocate subregions.
 * @param[in]  sz   Size of memory region pointed at by @a base.
 *
 * @retval AH_OK     if successful.
 * @retval AH_EINVAL if @a b is @c NULL or if @a base is @c NULL and @a sz is
 *                   not @c 0u.
 * @retval AH_ERANGE if aligning @a base and adding adding @a to it produces a
 *                   pointer that points beyond the end of the addressable
 *                   memory space.
 */
ah_err_t ah_bump_init(ah_bump_t* b, void* base, size_t sz);

/**
 * Allocates a subregion of memory from @a b with a size of @a sz bytes.
 *
 * @param[in,out] b  Pointer to bump allocator.
 * @param[in]     sz The smallest number of contiguous bytes that must be
 *                   possible to store in the memory allocated by this
 *                   operation.
 *
 * @return Pointer to beginning of allocated subregion, or @c NULL if not
 *         enough bytes remains in @a b to satisfy the request.
 *
 * @warning The given size @a sz will be rounded up to the nearest multiple of
 *          the size of a pointer, unless it already is a multiple of that
 *          size. This means that this call can fail even if it may seem as if
 *          the allocator has enough memory to satisfy the request.
 */
ahi_bump_alloc_attrs void* ah_bump_alloc(ah_bump_t* b, size_t sz);

/**
 * Resets referenced bump allocator @a b.
 *
 * Resetting entails the offset pointer of @a b being made equal to its base
 * pointer, which points to the beginning of the fixed-size memory region used
 * by the allocator.
 *
 * @param b Pointer to bump allocator to reset.
 */
ah_inline void ah_bump_reset(ah_bump_t* b)
{
    if (b != NULL) {
        b->off = b->base;
    }
}

/**
 * Reports the capacity, in bytes, of the referenced bump allocator @a b.
 *
 * @param b Pointer to bump allocator.
 *
 * @return Capacity of @a b, or @c 0u if @a b is @c NULL.
 */
ah_inline size_t ah_bump_get_capacity(const ah_bump_t* b)
{
    return b != NULL ? (size_t) (b->end - b->base) : 0u;
}

/**
 * Reports the space left, in bytes, in the referenced bump allocator @a b.
 *
 * @param b Pointer to bump allocator.
 *
 * @return Space left in @a b, or @c 0u if @a b is @c NULL.
 */
ah_inline size_t ah_bump_get_free_size(const ah_bump_t* b)
{
    return b != NULL ? (size_t) (b->end - b->off) : 0u;
}

/**
 * Reports the space used, in bytes, by the referenced bump allocator @a b.
 *
 * @param b Pointer to bump allocator.
 *
 * @return Space left in @a b, or @c 0u if @a b is @c NULL.
 */
ah_inline size_t ah_bump_get_used_size(const ah_bump_t* b)
{
    return b != NULL ? (size_t) (b->off - b->base) : 0u;
}

/** @} */

/**
 * @name Page Allocation
 *
 * The page is the smallest unit of memory, in bytes, that can be requested
 * directly from the platform operating system or memory manager. This page
 * allocator allows for these pages to be requested and returned directly.
 *
 * Page allocation is typically used to implement other kinds of allocation,
 * such as the @c malloc() function of C99 standard library and the slab
 * allocator of this library.
 *
 * @{
 */

/**
 * Allocates zero or more memory pages.
 *
 * Allocates the smallest number of contiguous pages required to store the
 * given @a sz number of bytes. The page is the smallest unit of memory that
 * can be requested directly from the platform operating system or memory
 * manager.
 *
 * @param[in] sz The smallest number of contiguous bytes that must be possible
 *               to store in the memory allocated by this operation.
 *
 * @return Pointer to the beginning of the allocated memory region, or @c NULL
 *         if the operation failed or if @a sz is @c 0u.
 */
ah_inline ahi_page_alloc_attrs void* ah_page_alloc(size_t sz)
{
    return ahp_page_alloc(sz);
}

/**
 * Frees the memory pages associated with the given @a ptr.
 *
 * @param[in,out] ptr Pointer received via a previous call to ah_page_alloc().
 * @param[in]     sz  The size provided to ah_page_alloc() when the memory
 *                    referred to by @a ptr was allocated.
 *
 * @warning The given `ptr` must have been received via a previous call to
 *          ah_page_alloc() and must not have been freed since then.
 */
ah_inline void ah_page_free(void* ptr, size_t sz)
{
    ahp_page_free(ptr, sz);
}

/**
 * Reports the size of a single memory page.
 *
 * @return The size, in bytes, of a single memory page.
 */
ah_inline size_t ah_page_get_size(void)
{
    return ahp_page_get_size();
}

/** @} */

/**
 * @name Slab Allocation
 *
 * A slab allocator maintains a variable number of fixed-size <em>slots</em>,
 * each of which can be allocated and freed very efficiently. Whenever the slab
 * allocator runs out of slots, it allocates a new <em>bank</em> of slots using
 * the page allocator. Allocated banks are not freed until the slab allocator
 * itself is terminated.
 *
 * In contrast to using something akin to the C99 @c realloc() function to grow
 * a chunk of memory, this method never requires already allocated slots to be
 * copied to new memory. This makes it safe to store pointers into these slots
 * as long as the slots and the slab allocator itself are not deallocated or
 * terminated. Another advantage of the slab allocator is that all of its
 * currently allocated slots can be deallocated at once by calling
 * ah_slab_term().
 *
 * @{
 */

/**
 * Initializes referenced slab allocator @a s for subsequent use.
 *
 * @param s       Pointer to slab to initialize.
 * @param slot_sz The smallest accepted size of each slab slot, in bytes.
 *
 * @retval AH_OK     if successful.
 * @retval AH_EINVAL if @a s is @c NULL.
 * @retval AH_ERANGE if the byte size of a slab slot or bank falls outside the
 *                   set of numbers that can be expressed by @c size_t.
 */
ah_err_t ah_slab_init(ah_slab_t* s, size_t slot_sz);

/**
 * Terminates referenced slab @a s, freeing all of its memory.
 *
 * If a callback function @a slot_cb_or_null is given, that function is called
 * once for each currently allocated slot before this function returns. It is
 * safe for the callback to call ah_slab_free() to free other slots allocated
 * with the same @a s. If such a freed slot has already been visited by the
 * callback, freeing it has no effect. If it has not yet been freed, however,
 * freeing it prevents it from being provided to the callback.
 *
 * @param s               Pointer to slab to terminate.
 * @param slot_cb_or_null Allocated slot callback function, or @c NULL.
 */
void ah_slab_term(ah_slab_t* s, void (*slot_cb_or_null)(void*));

/**
 * Allocates a free slot from the referenced slab @a s.
 *
 * If no free slots are available, an attempt will be made internally to
 * allocate another bank. If that attempt fails, @c NULL is returned.
 *
 * @param s Pointer to slab to allocate slot from.
 *
 * @return Pointer to beginning of allocated slot, or @c NULL if the allocation
 *         failed.
 */
ahi_slab_alloc_attrs void* ah_slab_alloc(ah_slab_t* s);

/**
 * Free a slot previously allocated from the same slab allocator @a s.
 *
 * The given @a ptr must have been received via a previous call to
 * ah_slab_alloc() with the same @a s and must not have been freed since then.
 *
 * @param s   Pointer to slab to allocate slot from.
 * @param ptr Pointer received via a previous call to ah_slab_alloc().
 */
void ah_slab_free(ah_slab_t* s, void* ptr);

/** @} */

#endif
