// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0.
//
// SPDX-License-Identifier: EPL-2.0

#include "ah/buf.h"

#include "ah/assert.h"

#include <string.h>

ah_extern uint8_t* ah_buf_get_base(ah_buf_t* buf)
{
    ah_assert_if_debug(buf != NULL);

    return (uint8_t*) buf->_base;
}

ah_extern const uint8_t* ah_buf_get_base_const(const ah_buf_t* buf)
{
    ah_assert_if_debug(buf != NULL);

    return (const uint8_t*) buf->_base;
}

ah_extern size_t ah_buf_get_size(const ah_buf_t* buf)
{
    ah_assert_if_debug(buf != NULL);

    return (size_t) buf->_size;
}

ah_extern bool ah_buf_is_empty(const ah_buf_t* buf)
{
    ah_assert_if_debug(buf != NULL);

    return buf->_base == NULL || buf->_size == 0u;
}

ah_extern void ah_buf_limit_size_to(ah_buf_t* buf, size_t limit)
{
    ah_assert_if_debug(buf != NULL);

    if (ah_buf_get_size(buf) > limit) {
        *buf = ah_buf_from(ah_buf_get_base(buf), (uint32_t) limit);
    }
}

ah_extern void ah_buf_rw_init_for_writing_to(ah_buf_rw_t* rw, ah_buf_t* buf)
{
    ah_assert_if_debug(rw != NULL);
    ah_assert_if_debug(buf != NULL);

    uint8_t* base = ah_buf_get_base(buf);
    uint8_t* end = &base[ah_buf_get_size(buf)]; // This is safe as long as `buf` is valid.

    *rw = (ah_buf_rw_t) {
        .rd = base,
        .wr = base,
        .end = end,
    };
}

ah_extern void ah_buf_rw_init_for_reading_from(ah_buf_rw_t* rw, const ah_buf_t* buf)
{
    ah_assert_if_debug(rw != NULL);
    ah_assert_if_debug(buf != NULL);

    uint8_t* base = (uint8_t*) ah_buf_get_base_const(buf);
    uint8_t* end = &base[ah_buf_get_size(buf)]; // This is safe as long as `buf` is valid.

    *rw = (ah_buf_rw_t) {
        .rd = base,
        .wr = (uint8_t*) end,
        .end = end,
    };
}

ah_extern void ah_buf_rw_get_readable_as_buf(const ah_buf_rw_t* rw, ah_buf_t* buf)
{
    ah_assert_if_debug(rw != NULL);
    ah_assert_if_debug(buf != NULL);

    *buf = (ah_buf_t) {
        ._base = (uint8_t*) rw->rd,
        ._size = (size_t) (rw->wr - rw->rd),
    };
}

ah_extern size_t ah_buf_rw_get_readable_size(const ah_buf_rw_t* rw)
{
    ah_assert_if_debug(rw != NULL);

    return (size_t) (rw->wr - rw->rd);
}

ah_extern void ah_buf_rw_get_writable_as_buf(const ah_buf_rw_t* rw, ah_buf_t* buf)
{
    ah_assert_if_debug(rw != NULL);
    ah_assert_if_debug(buf != NULL);

    *buf = (ah_buf_t) {
        ._base = (uint8_t*) rw->wr,
        ._size = (size_t) (rw->end - rw->wr),
    };
}

ah_extern size_t ah_buf_rw_get_writable_size(const ah_buf_rw_t* rw)
{
    ah_assert_if_debug(rw != NULL);

    return (size_t) (rw->end - rw->wr);
}

ah_extern bool ah_buf_rw_is_containing_buf(const ah_buf_rw_t* rw, const ah_buf_t* buf)
{
    ah_assert_if_debug(rw != NULL);
    ah_assert_if_debug(buf != NULL);

    const uint8_t* base = ah_buf_get_base_const(buf);
    return rw->rd <= base && rw->end >= &base[ah_buf_get_size(buf)];
}

ah_extern bool ah_buf_rw_copy1(ah_buf_rw_t* src, ah_buf_rw_t* dst)
{
    ah_assert_if_debug(src != NULL);
    ah_assert_if_debug(dst != NULL);

    if (ah_unlikely(src->wr == src->rd)) {
        return false;
    }

    if (ah_unlikely(dst->end == dst->wr)) {
        return false;
    }

    *dst->wr = *src->rd;
    src->rd = &src->rd[1u];
    dst->wr = &dst->wr[1u];

    return true;
}

ah_extern bool ah_buf_rw_copyn(ah_buf_rw_t* src, ah_buf_rw_t* dst, size_t size)
{
    ah_assert_if_debug(src != NULL);
    ah_assert_if_debug(dst != NULL);

    if (ah_unlikely((size_t) (src->wr - src->rd) < size)) {
        return false;
    }

    if (ah_unlikely((size_t) (dst->end - dst->wr) < size)) {
        return false;
    }

    memcpy(dst->wr, src->rd, size);
    src->rd = &src->rd[size];
    dst->wr = &dst->wr[size];

    return true;
}

ah_extern bool ah_buf_rw_peek1(ah_buf_rw_t* rw, uint8_t* dst)
{
    ah_assert_if_debug(rw != NULL);

    if (ah_unlikely(rw->rd == rw->wr)) {
        return false;
    }

    *dst = *rw->rd;

    return true;
}

ah_extern bool ah_buf_rw_peekn(ah_buf_rw_t* rw, uint8_t* dst, size_t size)
{
    ah_assert_if_debug(rw != NULL);
    ah_assert_if_debug(dst != NULL);

    if (ah_unlikely((size_t) (rw->wr - rw->rd) < size)) {
        return false;
    }

    memcpy(dst, rw->rd, size);

    return true;
}

ah_extern bool ah_buf_rw_read1(ah_buf_rw_t* rw, uint8_t* dst)
{
    ah_assert_if_debug(rw != NULL);

    if (ah_unlikely(rw->rd == rw->wr)) {
        return false;
    }

    *dst = *rw->rd;
    rw->rd = &rw->rd[1u];

    return true;
}

ah_extern bool ah_buf_rw_readn(ah_buf_rw_t* rw, uint8_t* dst, size_t size)
{
    ah_assert_if_debug(rw != NULL);
    ah_assert_if_debug(dst != NULL);

    if (ah_unlikely((size_t) (rw->wr - rw->rd) < size)) {
        return false;
    }

    memcpy(dst, rw->rd, size);
    rw->rd = &rw->rd[size];

    return true;
}

ah_extern bool ah_buf_rw_skip1(ah_buf_rw_t* rw)
{
    ah_assert_if_debug(rw != NULL);

    if (ah_unlikely(rw->wr == rw->rd)) {
        return false;
    }

    rw->rd = &rw->rd[1u];

    return true;
}

ah_extern bool ah_buf_rw_skipn(ah_buf_rw_t* rw, size_t size)
{
    ah_assert_if_debug(rw != NULL);

    if (ah_unlikely((size_t) (rw->wr - rw->rd) < size)) {
        return false;
    }

    rw->rd = &rw->rd[size];

    return true;
}

ah_extern bool ah_buf_rw_write(ah_buf_rw_t* rw, uint8_t* src, size_t size)
{
    ah_assert_if_debug(rw != NULL);
    ah_assert_if_debug(src != NULL);

    if (ah_unlikely((size_t) (rw->end - rw->wr) < size)) {
        return false;
    }

    memcpy(rw->wr, src, size);
    rw->wr = &rw->wr[size];

    return true;
}

ah_extern bool ah_buf_rw_write_byte(ah_buf_rw_t* rw, uint8_t byte)
{
    ah_assert_if_debug(rw != NULL);

    if (ah_unlikely(rw->wr == rw->end)) {
        return false;
    }

    *rw->wr = byte;
    rw->wr = &rw->wr[1u];

    return true;
}

ah_extern bool ah_buf_rw_write_cstr(ah_buf_rw_t* rw, const char* cstr)
{
    ah_assert_if_debug(rw != NULL);

    const uint8_t* c = (const uint8_t*) cstr;
    uint8_t* wr = rw->wr;

    while (wr != rw->end) {
        if (c[0u] == '\0') {
            rw->wr = wr;
            return true;
        }

        wr[0u] = c[0u];

        wr = &wr[1u];
        c = &c[1u];
    }

    return false;
}

ah_extern bool ah_buf_rw_write_size(ah_buf_rw_t* rw, size_t size)
{
    ah_assert_if_debug(rw != NULL);

    if (size == 0u)  {
        return ah_buf_rw_write_byte(rw, '0');
    }

    uint8_t buf[20]; // Large enough to hold all UINT64_MAX digits.
    uint8_t* off = &buf[sizeof(buf) - 1u];
    const uint8_t* end = off;

    size_t s = size;
    for (;;) {
        off[0u] = '0' + (s % 10);
        s /= 10;
        if (s == 0u) {
            break;
        }
        off = &off[-1];
    }

    return ah_buf_rw_write(rw, off, (size_t) (end - off));
}

ah_extern bool ah_buf_rw_juke1(ah_buf_rw_t* rw)
{
    ah_assert_if_debug(rw != NULL);

    if (ah_unlikely(rw->wr == rw->end)) {
        return false;
    }

    rw->wr = &rw->wr[1u];

    return true;
}

ah_extern bool ah_buf_rw_juken(ah_buf_rw_t* rw, size_t size)
{
    ah_assert_if_debug(rw != NULL);

    if (ah_unlikely((size_t) (rw->end - rw->wr) < size)) {
        return false;
    }

    rw->wr = &rw->wr[size];

    return true;
}
