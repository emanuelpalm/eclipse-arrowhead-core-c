// SPDX-License-Identifier: EPL-2.0

#ifndef AH_BUF_H
#define AH_BUF_H

/**
 * @file
 * Buffer representations.
 *
 * Types, functions and other utilities for handling buffers, which are
 * references to contiguous chunks of memory.
 */

#include "def.h"
#include "err.h"

#include <ahp/buf.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/**
 * Largest size, in bytes, that can be represented by an ah_buf_t instance.
 */
#define AH_BUF_SIZE_MAX AHP_BUF_SIZE_MAX

/**
 * Buffer cursor, useful for reading and writing from and to a buffer.
 *
 * Each such cursor consists of three pointers, pointing into a buffer as
 * follows:
 * @code
 *                           r           w                       e
 *                           |           |                       |
 *                           V           V                       V
 *               +---+---+---+---+---+---+---+---+---+---+---+---+
 * Buffer memory | 1 | 7 | 3 | 2 | 4 | 1 | 0 | 0 | 0 | 0 | 0 | 0 |
 *               +---+---+---+---+---+---+---+---+---+---+---+---+
 *                            :.........: :.....................:
 *                                 :                 :
 *                          Readable bytes     Writable bytes
 * @endcode
 *
 * The following must always be true for every buffer cursor in use:
 * <ol>
 *   <li>The @c r pointer must be less than or equal to @c w.
 *   <li>The @c w pointer must be less than or equal to @c e.
 *   <li>All of @c r, @c w and @c e must point into or to the first byte right
 *       after the same contiguous region of buffer memory.
 * </ol>
 *
 * The most straightforward way to ensure that the invariants remain true for a
 * given buffer cursor instance is to initialize and update it only through the
 * functions for handling such cursors available in this library.
 */
struct ah_bufc {
    uint8_t* r; ///< Points to next unread byte.
    uint8_t* w; ///< Points to next unwritten byte.
    uint8_t* e; ///< Points to first byte outside of the cursor's memory chunk.
};

/**
 * Initializes @a b with @a base pointer and @a sz.
 *
 * @param[out] b    Pointer to buffer to initialize.
 * @param[in]  base Pointer to chunk of memory.
 * @param[in]  sz   Size of chunk of memory referred to by @a base.
 *
 * @retval AH_OK        if successful.
 * @retval AH_EINVAL    if @a b is @c NULL or @a base is @c NULL and @a sz is
 *                      positive.
 * @retval AH_EOVERFLOW if @a sz is larger than @c AH_BUF_SIZE_MAX.
 */
ah_inline ah_err_t ah_buf_init(ah_buf_t* b, uint8_t* base, size_t sz)
{
    return ahp_buf_init(b, base, sz);
}

/**
 * Initializes @a b from the readable region of bytes in @a c.
 *
 * @param[out] b Pointer to buffer to initialize.
 * @param[in]  c Pointer to cursor to get readable bytes from.
 *
 * @retval AH_OK        Initialization was successful.
 * @retval AH_EINVAL    @a b or @a c is @c NULL.
 * @retval AH_EOVERFLOW The readable region of @a c is larger than
 *                      @c AH_BUF_SIZE_MAX.
 */
ah_inline ah_err_t ah_buf_init_from_readable(ah_buf_t* b, ah_bufc_t* c)
{
    if (c == NULL) {
        return AH_EINVAL;
    }
    return ah_buf_init(b, c->r, c->w - c->r);
}

/**
 * Initializes @a b from the writable region of bytes in @a c.
 *
 * @param[out] b Pointer to buffer to initialize.
 * @param[in]  c Pointer to cursor to get writable bytes from.
 *
 * @retval AH_OK        if successful.
 * @retval AH_EINVAL    if @a b or @a c is @c NULL.
 * @retval AH_EOVERFLOW if the writable region of @a c is larger than
 *                      @c AH_BUF_SIZE_MAX.
 */
ah_inline ah_err_t ah_buf_init_from_writable(ah_buf_t* b, ah_bufc_t* c)
{
    if (c == NULL) {
        return AH_EINVAL;
    }
    return ah_buf_init(b, c->w, c->e - c->w);
}

/**
 * Gets pointer to beginning of memory chunk referred to by this buffer.
 *
 * @param[in] b Pointer to buffer.
 *
 * @return Pointer to memory chunk, or @c NULL if @a b is @c NULL or if @a b
 *         has @c NULL as its base pointer.
 */
ah_inline uint8_t* ah_buf_get_base(ah_buf_t* b)
{
    return ahp_buf_get_base(b);
}

/**
 * Gets size of the memory chunk referred to by this buffer.
 *
 * @param[in] b Pointer to buffer.
 *
 * @return Size of memory chunk, or @c 0u if @a b is @c NULL or if @a b is
 *         referring to a memory chunk with size @c 0u.
 */
ah_inline size_t ah_buf_get_sz(ah_buf_t* b)
{
    return ahp_buf_get_sz(b);
}

/**
 * Checks if @a b has a @c NULL @c base or a @c size being @c 0u.
 *
 * @param[in] b Pointer to checked buffer.
 *
 * @return @c true, only if @a b is @c NULL, has a @c NULL @c base or @c a size
 *         being @c 0u. @c false otherwise.
 */
ah_inline bool ah_buf_is_empty(ah_buf_t* b)
{
    return b == NULL || ah_buf_get_base(b) == NULL || ah_buf_get_sz(b) == 0u;
}

/**
 * Creates new readable cursor from @a base and @a sz.
 *
 * The cursor created by this function will treat @a base as initialized memory
 * and only allow for it to be read.
 *
 * @param[in] base Pointer to first byte of a readable memory chunk.
 * @param[in] sz   The number of readable bytes referred to by @a base.
 *
 * @return Created cursor.
 *
 * @note If @a base is NULL or @a sz is @c 0u, a zeroed cursor is returned.
 */
ah_inline ah_bufc_t ah_bufc_from_readable(const void* base, size_t sz)
{
    if (base == NULL || sz == 0u) {
        return (ah_bufc_t) { 0u };
    }
    return (ah_bufc_t) {
        .r = &((uint8_t*) base)[0u],
        .w = &((uint8_t*) base)[sz],
        .e = &((uint8_t*) base)[sz],
    };
}

/**
 * Creates new readable cursor from @a b.
 *
 * The cursor created by this function will treat the memory referred to by
 * @a b as initialized memory and only allow for it to be read.
 *
 * @param[in] b Pointer to a buffer.
 *
 * @return Created cursor.
 *
 * @note Returns a zeroed ah_bufc if @a b is @c NULL.
 *
 * @warning If @a b is invalid, also the created ah_bufc becomes invalid.
 */
ah_inline ah_bufc_t ah_bufc_from_readable_buf(ah_buf_t* b)
{
    return ah_bufc_from_readable(ah_buf_get_base(b), ah_buf_get_sz(b));
}

/**
 * Creates new writable cursor from @a base and @a sz.
 *
 * The cursor created by this function will treat @a base as a pointer to
 * uninitialized memory and require that it is written to before it can be read.
 *
 * @param[in] base Pointer to first byte of a writable memory chunk.
 * @param[in] sz   The number of writable bytes referred to by @a base.
 *
 * @return Created cursor.
 *
 * @note If @a base is NULL or @a sz is @c 0u, a zeroed cursor is returned.
 */
ah_inline ah_bufc_t ah_bufc_from_writable(void* base, size_t sz)
{
    if (base == NULL || sz == 0u) {
        return (ah_bufc_t) { 0u };
    }
    return (ah_bufc_t) {
        .r = &((uint8_t*) base)[0u],
        .w = &((uint8_t*) base)[0u],
        .e = &((uint8_t*) base)[sz],
    };
}

/**
 * Creates new writable cursor from @a b.
 *
 * The cursor created by this function will treat the memory referred to by
 * @a b as uninitialized memory and require that it is written to before it can
 * be read.
 *
 * @param[in] b Pointer to a buffer.
 *
 * @return Created cursor.
 *
 * @note If @a base is NULL and @a sz is positive, a zeroed cursor is returned.
 */
ah_inline ah_bufc_t ah_bufc_from_writable_buf(ah_buf_t* b)
{
    return ah_bufc_from_writable(ah_buf_get_base(b), ah_buf_get_sz(b));
}

/**
 * Calculates the size of the readable range of @a c.
 *
 * @param[in] c Pointer to cursor.
 *
 * @return Number of bytes part of the readable memory referred to by @a c.
 *
 * @note Returns @c 0u if @a c is @c NULL.
 */
ah_inline size_t ah_bufc_get_readable_sz(const ah_bufc_t* c)
{
    return c == NULL ? 0u : (c->w - c->r);
}

/**
 * Calculates the size of the writable range of @a c.
 *
 * @param[in] c Pointer to cursor.
 *
 * @return Number of bytes part of the writable memory referred to by @a c.
 *
 * @note Returns @c 0 if @a c is @c NULL.
 */
ah_inline size_t ah_bufc_get_writable_sz(const ah_bufc_t* c)
{
    return c == NULL ? 0u : (c->e - c->w);
}

/**
 * Checks if the readable section of @a c has a non-zero size.
 *
 * @param[in] c Pointer to cursor.
 *
 * @return @c true only if @a c refers to a non-zero number of readable bytes.
 *
 * @note Returns @c false if @a c is @c NULL.
 */
ah_inline bool ah_bufc_is_readable(const ah_bufc_t* c)
{
    return c != NULL && c->r < c->w;
}

/**
 * Checks if the writable section of @a c has a non-zero size.
 *
 * @param[in] c Pointer to cursor.
 *
 * @return @c true only if @a c refers to a non-zero number of writable bytes.
 *
 * @note Returns @c false if @a c is @c NULL.
 */
ah_inline bool ah_bufc_is_writable(const ah_bufc_t* c)
{
    return c != NULL && c->w < c->e;
}

/**
 * Reads one byte from @a src and writes it to @a dst.
 *
 * @param[in,out] src Pointer to source cursor.
 * @param[in,out] dst Pointer to destination cursor.
 *
 * @return @c true only if exactly one byte could be read from @a src and be
 *         written to @a dst.
 *
 * @note Does nothing and returns @c false if either of @a src or @a dst is
 *       @c NULL, if @a src has no readable byte, or if @a dst hos no writable
 *       byte.
 */
ah_inline bool ah_bufc_copy_1(ah_bufc_t* src, ah_bufc_t* dst)
{
    if (src == NULL || src->r == src->w || dst == NULL || dst->w == dst->e) {
        return false;
    }
    dst->w[0u] = src->r[0u];
    src->r = &src->r[1u];
    dst->w = &dst->w[1u];
    return true;
}

/**
 * Reads @a n bytes from @a src and writes them to @a dst.
 *
 * @param[in,out] src Pointer to source cursor.
 * @param[in,out] dst Pointer to destination cursor.
 * @param[in]     n   Number of bytes to copy from @a src to @a dst.
 *
 * @return @c true only if exactly @a n bytes could be read from @a src and be
 *         written to @a dst.
 *
 * @note Does nothing and returns @c false if either of @a src or @a dst is
 *       @c NULL, if less than @a n bytes can be read from @a src, or if less
 *       than @a n bytes can be written to @a dst.
 */
ah_inline bool ah_bufc_copy_n(ah_bufc_t* src, ah_bufc_t* dst, size_t n)
{
    if (src == NULL || (size_t) (src->w - src->r) < n) {
        return false;
    }
    if (dst == NULL || (size_t) (dst->e - dst->w) < n) {
        return false;
    }
    memcpy(dst->w, src->r, n);
    src->r = &src->r[n];
    dst->w = &dst->w[n];
    return true;
}

/**
 * Reads one byte from @a c and writes it to @a dst.
 *
 * @param[in]  c   Pointer to source cursor.
 * @param[out] dst Pointer to byte receiver.
 *
 * @return @c true only if exactly one byte could be read from @a c and be
 *         written to @a dst.
 *
 * @note Does nothing and returns @c false if either of @a c or @a dst is
 *       @c NULL, or if @a c has no readable byte.
 */
ah_inline bool ah_bufc_peek_1(ah_bufc_t* c, uint8_t* dst)
{
    if (c == NULL || c->r == c->w || dst == NULL) {
        return false;
    }
    *dst = c->r[0u];
    return true;
}

/**
 * Reads @a n bytes from @a src and writes them to @a dst.
 *
 * @param[in]  c   Pointer to source cursor.
 * @param[out] dst Pointer to beginning of memory chunk that will receive copy
 *                 of the read bytes.
 * @param[in]  n   Number of bytes to read from @a c and write to @a dst.
 *
 * @return @c true only if exactly @a n bytes could be read from @a c and be
 *         written to @a dst.
 *
 * @note Does nothing and returns @c false if either of @a src or @a dst is
 *       @c NULL, or if @a c has less than @a n readable bytes.
 */
ah_inline bool ah_bufc_peek_n(ah_bufc_t* c, uint8_t* dst, size_t n)
{
    if (c == NULL || (size_t) (c->w - c->r) < n || dst == NULL) {
        return false;
    }
    memmove(dst, c->r, n);
    return true;
}

/**
 * Reads one byte from @a c, writes it to @a dst and advances the read pointer
 * of @a c one byte.
 *
 * @param[in,out] c   Pointer to source cursor.
 * @param[out]    dst Pointer to byte receiver.
 *
 * @return @c true only if exactly one byte could be read from @a c and be
 *         written to @a dst.
 *
 * @note Does nothing and returns @c false if @a c is @c NULL or @a c has no
 *       more readable bytes.
 */
ah_inline bool ah_bufc_read_1(ah_bufc_t* c, uint8_t* dst)
{
    if (c == NULL || c->r == c->w || dst == NULL) {
        return false;
    }
    *dst = c->r[0u];
    c->r = &c->r[1u];
    return true;
}

/**
 * Reads @a n bytes from @a c, writes them to @a dst and advances the read
 * pointer of @a c @a n bytes.
 *
 * @param[in,out] c   Pointer to source cursor.
 * @param[out]    dst Pointer to beginning of memory chunk that will receive
 *                    copy of the read bytes.
 * @param[in]     n   Number of bytes to read from @a c and write to @a dst.
 *
 * @return @c true only if exactly @a n bytes could be read from @a c and be
 *         written to @a dst.
 *
 * @note Does nothing and returns @c false if either of @a c or @a dst is
 *       @c NULL, or if less than @a n bytes can be read from @a c.
 */
ah_inline bool ah_bufc_read_n(ah_bufc_t* c, uint8_t* dst, size_t n)
{
    if (c == NULL || (size_t) (c->w - c->r) < n || dst == NULL) {
        return false;
    }
    memmove(dst, c->r, n);
    c->r = &c->r[n];
    return true;
}

/**
 * Advances the read pointer of @a c one byte.
 *
 * @param[in,out] c Pointer to cursor.
 *
 * @return @c true only if exactly one byte could be read and discarded from
 *         @a c.
 *
 * @note Does nothing and returns @c false if @a src is @c NULL, or if @a c
 *       has no more readable bytes.
 */
ah_inline bool ah_bufc_skip_1(ah_bufc_t* c)
{
    if (c == NULL || c->r == c->w) {
        return false;
    }
    c->r = &c->r[1u];
    return true;
}

/**
 * Advances the read pointer of @a c @a n bytes.
 *
 * @param[in,out] c Pointer to cursor.
 * @param[in]     n Number of bytes to skip.
 *
 * @return @c true only if exactly @a n bytes could be read and discarded from
 *         @a c.
 *
 * @note Does nothing and returns @c false if @a c is @c NULL or if @a n is
 *       larger than the number of readable bytes in @a c.
 */
ah_inline bool ah_bufc_skip_n(ah_bufc_t* c, size_t n)
{
    if (c == NULL || (size_t) (c->w - c->r) < n) {
        return false;
    }
    c->r = &c->r[n];
    return true;
}

/**
 * Advances the read pointer of @a c to its write pointer, effectively
 * discarding all currently readable bytes.
 *
 * @param[in,out] c Pointer to cursor.
 *
 * @note Does nothing if @a c is @c NULL.
 */
ah_inline void ah_bufc_skip_all(ah_bufc_t* c)
{
    if (c != NULL) {
        c->r = c->w;
    }
}

/**
 * Writes @a u to @a c and advances its write pointer one byte.
 *
 * @param[in,out] c Pointer to destination cursor.
 * @param[in]     u Byte to write.
 *
 * @return @c true only if @a u could be written to @a c.
 *
 * @note Does nothing and returns @c false if @a c is @c NULL or if @a c has
 *       no more writable bytes.
 */
ah_inline bool ah_bufc_write_1(ah_bufc_t* c, uint8_t u)
{
    if (c == NULL || c->w == c->e) {
        return false;
    }
    c->w[0u] = u;
    c->w = &c->w[1u];
    return true;
}

/**
 * Writes formatted string to @a c and advances its write pointer to right after
 * the written string.
 *
 * Internally, this function uses the C99 @c vsnprintf() function, which means
 * that it accepts the same conversion specifications.
 *
 * @param[in,out] c   Pointer to destination cursor.
 * @param[in]     fmt Format string.
 * @param[in]     ... Format arguments.
 *
 * @return @c true only if all of the formatted string could be written to @a c.
 *
 * @note Writes an additional `\0` right after the end of the formatted string
 *       without advancing the write pointer, but only if there is room for it.
 *
 * @note Does nothing and returns @c false if either of @a c or @a fmt is
 *       @c NULL. If the internal call to @c vsnprintf() fails, or if not all of
 *       its output could be written, then writable memory in @a c may have been
 *       written to without its write pointer having been updated.
 */
ah_inline bool ah_bufc_write_fmt(ah_bufc_t* c, const char* fmt, ...)
{
    if (c == NULL || fmt == NULL) {
        return false;
    }

    size_t sz = ah_bufc_get_writable_sz(c);

    va_list args;
    va_start(args, fmt);
    int res = vsnprintf((char*) c->w, sz, fmt, args);
    va_end(args);

    if (res < 0 || ((uintmax_t) res) > ((uintmax_t) sz)) {
        return false;
    }

    c->w = &c->w[res];

    return true;
}

/**
 * Writes @a n bytes from @a src to @a c and advances its write pointer @a n
 * bytes.
 *
 * @param[in,out] c   Pointer to destination cursor.
 * @param[in]     src Pointer to beginning of memory chunk that contains the
 *                    bytes that are to be written to @a c.
 * @param[in] n       Number of bytes to write.
 *
 * @return @c true only if @a n bytes from @a src could be written to @a c.
 *
 * @note Does nothing and returns @c false if either of @a c or @a src is
 *       @c NULL, or if @a n is larger than the number of writable bytes in
 *       @a c.
 */
ah_inline bool ah_bufc_write_n(ah_bufc_t* c, uint8_t* src, size_t n)
{
    if (c == NULL || (src == NULL && n > 0u) || (size_t) (c->e - c->w) < n) {
        return false;
    }
    memmove(c->w, src, n);
    c->w = &c->w[n];
    return true;
}

/**
 * Writes @a n bytes of void, or nothing, to @a c and advances its write
 * pointer @a n bytes.
 *
 * Writing void, or nothing, is the same as leaving the concerned bytes as they
 * are. This operation is useful when needing to account for that data was
 * written to the buffer of @a c via some other means than the functions of
 * this library, such as the @c snprintf() C99 function.
 *
 * @param[in,out] c Pointer to cursor.
 * @param[in]     n Number of bytes to skip.
 *
 * @return @c true only if the write pointer of @a c could be advanced @a n
 *         bytes.
 *
 * @note Does nothing and returns @c false if @a c is @c NULL or if @a n is
 *       larger than the remaining number of writable bytes in @a c.
 */
ah_inline bool ah_bufc_write_v(ah_bufc_t* c, size_t n)
{
    if (c == NULL || (size_t) (c->e - c->w) < n) {
        return false;
    }
    c->w = &c->w[n];
    return true;
}

#endif
