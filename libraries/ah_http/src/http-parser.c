// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0.
//
// SPDX-License-Identifier: EPL-2.0

#include "http-parser.h"

#include <ah/err.h>
#include <ah/math.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

void ah_i_http_rwbuf_init(struct ah_i_http_rwbuf* rw, const uint8_t* base, unsigned long size)
{
    ah_assert_if_debug(rw != NULL);
    *rw = (struct ah_i_http_rwbuf) { base, base, &base[size] };
}

ah_err_t ah_i_http_rwbuf_migrate_to(struct ah_i_http_rwbuf* rw, ah_buf_t* target)
{
    ah_assert_if_debug(rw != NULL);
    ah_assert_if_debug(target != NULL);

    ah_assert_if_debug(rw->_end >= rw->_rd);
    size_t rem = rw->_end - rw->_rd;

    if (rem > ah_buf_get_size(target)) {
        return AH_ENOBUFS;
    }

    (void) memcpy(ah_buf_get_base(target), rw->_rd, rem);

    ah_err_t err = ah_buf_init(target, )
}

/*
static struct ah_i_http_rwbuf s_reader_from_buf_and_size(ah_buf_t* buf, size_t size);
static void s_reader_into_buf_and_size(struct ah_i_http_rwbuf* r, ah_buf_t* buf, size_t* size);

static bool s_parse_status_code(struct ah_i_http_rwbuf* r, uint16_t* code);
static bool s_parse_version(struct ah_i_http_rwbuf* r, ah_http_ver_t* version);

static bool s_is_digit(uint8_t ch);
static bool s_is_ows(uint8_t ch);
static bool s_is_rchar(uint8_t ch);
static bool s_is_tchar(uint8_t ch);
static bool s_is_vchar_obs_text_htab_sp(uint8_t ch);
static bool s_is_vchar_obs_text(uint8_t ch);

static bool s_skip_ch(struct ah_i_http_rwbuf* r, char ch);
static bool s_skip_crlf(struct ah_i_http_rwbuf* r);
static void s_skip_ows(struct ah_i_http_rwbuf* r);

static ah_str_t s_take_while(struct ah_i_http_rwbuf* r, bool (*pred)(uint8_t));

struct ah_i_http_rwbuf ah_i_http_parser_from(const ah_buf_t* buf, size_t limit)
{
    ah_assert_if_debug(buf != NULL);
    ah_assert_if_debug(ah_buf_get_size(buf) >= limit);

    const uint8_t* off = ah_buf_get_base_const(buf);
    const uint8_t* const end = &off[limit];

    return (struct ah_i_http_rwbuf) { off, end };
}

void ah_i_http_reader_into_buf(const struct ah_i_http_rwbuf* r, ah_buf_t* buf) {
    ah_assert_if_debug(r != NULL);
    ah_assert_if_debug(buf != NULL);

    ah_buf_from(r->_off_rd, r->end - r->_off_rd);
}

ah_err_t ah_i_http_skip_until_after_line_end(ah_buf_t* src, size_t* size)
{
    ah_assert_if_debug(src != NULL);
    ah_assert_if_debug(size != NULL);
    ah_assert_if_debug(ah_buf_get_size(src) >= *size);

    uint8_t* const beg = ah_buf_get_base(src);
    uint8_t* off = beg;
    uint8_t* const end = &off[*size];

    ah_err_t err = AH_EAGAIN;

    for (; off != end; off = &off[1u]) {
        if (off[0u] != '\r') {
            continue;
        }
        if (&off[1u] == end) {
            break;
        }
        if (off[1u] != '\n') {
            err = AH_EILSEQ;
            break;
        }
        off = &off[2u];
        err = AH_ENONE;
        break;
    }

    const size_t nread = off - beg;
    *size -= nread;
    ah_buf_from(off, ah_buf_get_size(src) - nread);

    return err;
}

ah_err_t ah_i_http_skip_until_after_headers_end(ah_buf_t* src, size_t* size)
{
    ah_assert_if_debug(src != NULL);
    ah_assert_if_debug(size != NULL);
    ah_assert_if_debug(ah_buf_get_size(src) >= *size);

    uint8_t* const beg = ah_buf_get_base(src);
    uint8_t* off = beg;
    uint8_t* const end = &off[*size];

    ah_err_t err = AH_EAGAIN;

    for (; off != end; off = &off[1u]) {
        if (off[0u] != '\r') {
            continue;
        }
        if (&off[1u] == end) {
            break;
        }
        if (off[1u] != '\n') {
            err = AH_EILSEQ;
            break;
        }
        if (&off[2u] == end) {
            break;
        }
        if (off[2u] != '\r') {
            off = &off[2u];
            continue;
        }
        if (&off[3u] == end) {
            break;
        }
        if (off[3u] != '\n') {
            err = AH_EILSEQ;
            break;
        }
        off = &off[4u];
        err = AH_ENONE;
        break;
    }

    const size_t nread = off - beg;
    *size -= nread;
    ah_buf_from(off, ah_buf_get_size(src) - nread);

    return err;
}

ah_err_t ah_i_http_parse_chunk(ah_buf_t* src, size_t* size, ah_http_chunk_t* chunk)
{
    ah_assert_if_debug(src != NULL);
    ah_assert_if_debug(size != NULL);
    ah_assert_if_debug(ah_buf_get_size(src) >= *size);
    ah_assert_if_debug(chunk != NULL);

    uint8_t* const beg = ah_buf_get_base(src);
    uint8_t* off = beg;
    uint8_t* const end = &off[*size];

    ah_err_t err;

    size_t chunk_size = 0u;
    ah_str_t ext = (ah_str_t) { 0u };
    size_t nread;

    uint8_t* ext_beg;

    for (; off != end; off = &off[1u]) {
        const uint8_t ch = off[0u];

        size_t inc;
        if (ch >= '0' && ch <= '9') {
            inc = ch - '0';
        }
        else if (ch >= 'A' && ch <= 'F') {
            inc = (ch - 'A') + 10u;
        }
        else if (ch >= 'a' && ch <= 'f') {
            inc = (ch - 'a') + 10u;
        }
        else if (ch == '\r' && off != end && off[1u] == '\n') {
            off = &off[1u];
            goto finish;
        }
        else if (ch == ';') {
            goto parse_ext;
        }
        else {
            err = AH_EILSEQ;
            goto finish;
        }

        err = ah_mul_size(chunk_size, 16u, &chunk_size);
        if (err != AH_ENONE) {
            goto finish;
        }

        err = ah_add_size(chunk_size, inc, &chunk_size);
        if (err != AH_ENONE) {
            goto finish;
        }
    }

    err = AH_EEOF;
    goto finish;

parse_ext:

    ext_beg = off;

    for (; off != end; off = &off[1u]) {
        if (off[0u] != '\r') {
            continue;
        }
        off = &off[1u];
        if (off == end) {
            err = AH_EEOF;
            goto finish;
        }
        if (off[0u] != '\n') {
            err = AH_EILSEQ;
            goto finish;
        }
        break;
    }

    ext = ah_str_from(ext_beg, &end[-2] - ext_beg);

finish:

    nread = off - beg;
    *size -= nread;
    ah_buf_from(off, ah_buf_get_size(src) - nread);

    *chunk = (ah_http_chunk_t) {
        .size = chunk_size,
        .ext = ext,
    };

    return err;
}

ah_err_t ah_i_http_parse_headers(ah_buf_t* src, size_t* size, ah_http_hmap_t* hmap)
{
    ah_assert_if_debug(src != NULL);
    ah_assert_if_debug(hmap != NULL);

    struct ah_i_http_rwbuf r = s_reader_from_buf_and_size(src, 0);

    for (;;) {
        if (s_skip_crlf(&r)) {
            return AH_ENONE;
        }

        // Read name.

        ah_str_t name = s_take_while(&r, s_is_tchar);
        if (ah_str_get_len(&name) == 0u || !s_skip_ch(&r, ':')) {
            return AH_EILSEQ;
        }

        // Read value.

        s_skip_ows(&r);

        if (r._off_rd == r.end || !s_is_vchar_obs_text(r._off_rd[0u])) {
            return AH_EILSEQ;
        }

        uint8_t* field_value_start = r._off_rd;

        do {
            r._off_rd = &r._off_rd[1u];
        } while (s_is_vchar_obs_text_htab_sp(r._off_rd[0u]));

        uint8_t* field_value_end = r._off_rd;

        if (!s_skip_crlf(&r)) {
            return AH_EILSEQ;
        }

        // Remove trailing optional whitespace.
        while (field_value_end != field_value_start && s_is_ows(field_value_end[-1])) {
            field_value_end = &field_value_end[-1];
        }

        ah_str_t value = ah_str_from(field_value_start, field_value_end - field_value_start);

        ah_err_t err = ah_http_hmap_add(hmap, name, value);
        if (err != AH_ENONE) {
            return err;
        }
    }
}

ah_err_t ah_i_http_parse_req_line(ah_buf_t* src, size_t* size, ah_http_req_line_t* req_line)
{
    ah_assert_if_debug(src != NULL);
    ah_assert_if_debug(req_line != NULL);

    struct ah_i_http_rwbuf r = s_reader_from_buf_and_size(src, 0);

    req_line->method = s_take_while(&r, s_is_tchar);
    if (ah_str_get_len(&req_line->method) == 0u || !s_skip_ch(&r, ' ')) {
        return false;
    }

    req_line->target = s_take_while(&r, s_is_rchar);
    if (ah_str_get_len(&req_line->target) == 0u || !s_skip_ch(&r, ' ')) {
        return false;
    }

    if (!s_parse_version(&r, &req_line->version) || !s_skip_crlf(&r)) {
        return false;
    }

    (void) ah_buf_init(src, r._off_rd, r.end - r._off_rd);

    return true;
}

static bool s_parse_version(struct ah_i_http_rwbuf* r, ah_http_ver_t* version)
{
    if (r->end - r->_off_rd < 8u) {
        return false;
    }
    if (memcmp(r->_off_rd, "HTTP/", 5u) != 0) {
        return false;
    }
    if (!s_is_digit(r->_off_rd[5u]) || r->_off_rd[6u] != '.' || !s_is_digit(r->_off_rd[7u])) {
        return false;
    }

    version->major = r->_off_rd[5u] - '0';
    version->minor = r->_off_rd[7u] - '0';

    r->_off_rd = &r->_off_rd[8u];

    return true;
}

ah_err_t ah_i_http_parse_stat_line(ah_buf_t* src, size_t* size, ah_http_stat_line_t* stat_line)
{
    ah_assert_if_debug(src != NULL);
    ah_assert_if_debug(stat_line != NULL);

    struct ah_i_http_rwbuf r = s_reader_from_buf_and_size(src, 0);

    if (!s_parse_version(&r, &stat_line->version) || !s_skip_ch(&r, ' ')) {
        return false;
    }

    if (!s_parse_status_code(&r, &stat_line->code) || !s_skip_ch(&r, ' ')) {
        return false;
    }

    stat_line->reason = s_take_while(&r, s_is_vchar_obs_text_htab_sp);

    if (!s_skip_crlf(&r)) {
        return false;
    }

    (void) ah_buf_init(src, r._off_rd, r.end - r._off_rd);

    return true;
}

static bool s_parse_status_code(struct ah_i_http_rwbuf* r, uint16_t* code)
{
    if (r->end - r->_off_rd < 3u) {
        return false;
    }
    if (!s_is_digit(r->_off_rd[0u]) || !s_is_digit(r->_off_rd[1u]) || !s_is_digit(r->_off_rd[2u])) {
        return false;
    }

    *code = ((r->_off_rd[0u] - '0') * 100) + ((r->_off_rd[1u] - '0') * 10) + (r->_off_rd[2u] - '0');

    r->_off_rd = &r->_off_rd[3u];

    return true;
}

static struct ah_i_http_rwbuf s_reader_from_buf_and_size(ah_buf_t* buf, size_t size)
{
    ah_assert_if_debug(buf != NULL);
    ah_assert_if_debug(ah_buf_get_size(buf) >= size);

    uint8_t* off = ah_buf_get_base(buf);
    return (struct ah_i_http_rwbuf) { off, &off[size] };
}

static void s_reader_into_buf_and_size(struct ah_i_http_rwbuf* r, ah_buf_t* buf, size_t* size)
{
    ah_assert_if_debug(r != NULL);
    ah_assert_if_debug(buf != NULL);
    ah_assert_if_debug(size != NULL);



    (void) ah_buf_init(buf, r->_off_rd, r->end - r->_off_rd);
}

static bool s_is_digit(uint8_t ch)
{
    return ch >= '0' && ch <= '9';
}

static bool s_is_ows(uint8_t ch)
{
    return ch == ' ' || ch == '\t';
}

static bool s_is_rchar(uint8_t ch)
{
    // Every set bit in this table denotes a character that may occur in an
    // RFC7230 request-target. Those characters are '!', '$', '%', '&', '\'',
    // '(', ')', '*', '+', ',', '-', '.', '/', [0-9], ':', ';', '=', '?', '@',
    // [A-Z], '[', ']', '_', [a-z] and '~'.
    static const uint32_t tab[] = {
        0x00000000,
        0xAFFFFFF2,
        0xAFFFFFFF,
        0x47FFFFFE,
    };
    return (ch & 0x80) == 0u && ((tab[ch >> 5u] >> (ch & 0x1F)) & 1u) == 1u;
}

static bool s_is_tchar(uint8_t ch)
{
    // Every set bit in this table denotes a token character (TCHAR) of RFC7230.
    // Those characters are '!', '#', '$', '%', '&', '\'', '*', '+', '-', '.',
    // [0-9], [A-Z], '^', '_', '`', [a-z], '|' and '~'.
    static const uint32_t tab[] = {
        0x00000000,
        0x03FF6CFA,
        0xC7FFFFFE,
        0x57FFFFFE,
    };
    return (ch & 0x80) == 0u && ((tab[ch >> 5u] >> (ch & 0x1F)) & 1u) == 1u;
}

static bool s_is_vchar_obs_text(uint8_t ch)
{
    return ch > 0x20 && ch != 0x7F;
}

static bool s_is_vchar_obs_text_htab_sp(uint8_t ch)
{
    return (ch >= 0x20 && ch != 0x7F) || ch == '\t';
}

static bool s_skip_ch(struct ah_i_http_rwbuf* r, char ch)
{
    if (r->_off_rd == r->end || r->_off_rd[0u] != ch) {
        return false;
    }

    r->_off_rd = &r->_off_rd[1u];

    return true;
}

static bool s_skip_crlf(struct ah_i_http_rwbuf* r)
{
    if ((size_t) (r->end - r->_off_rd) < 2u) {
        return false;
    }
    if (memcmp(r->_off_rd, (uint8_t[]) { '\r', '\n' }, 2u) != 0) {
        return false;
    }

    r->_off_rd = &r->_off_rd[2u];

    return true;
}

static void s_skip_ows(struct ah_i_http_rwbuf* r)
{
    while (r->_off_rd != r->end && s_is_ows(r->_off_rd[0u])) {
        r->_off_rd = &r->_off_rd[1u];
    }
}

static ah_str_t s_take_while(struct ah_i_http_rwbuf* r, bool (*pred)(uint8_t))
{
    uint8_t* off = r->_off_rd;

    for (; off != r->end; off = &off[1u]) {
        if (!pred(off[0u])) {
            break;
        }
    }

    uint8_t* ptr = r->_off_rd;
    size_t len = (size_t) (off - r->_off_rd);

    r->_off_rd = off;

    return ah_str_from(ptr, len);
}
*/
