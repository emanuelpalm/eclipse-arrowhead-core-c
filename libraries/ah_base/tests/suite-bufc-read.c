// SPDX-License-Identifier: EPL-2.0

#include "ah/bit.h"
#include "ah/buf.h"

#include <ah/unit.h>

AH_UNIT_SUITE(bufc_read)
{
    AH_UNIT_TEST("ah_bufc_read() rejects invalid arguments.")
    {
        uint8_t buffer[32u];

        AH_UNIT_CASE("`c` is NULL.")
        {
            AH_UNIT_EQ_BOOL(false, ah_bufc_read(NULL, buffer, sizeof(buffer)));
        }

        AH_UNIT_CASE("`dst` is NULL.")
        {
            ah_bufc_t src = ah_bufc_from_readable(buffer, sizeof(buffer));
            AH_UNIT_EQ_BOOL(false, ah_bufc_read(&src, NULL, 1u));
        }

        AH_UNIT_CASE("`n` is too large for `c`.")
        {
            ah_bufc_t src = ah_bufc_from_readable(NULL, 0u);
            AH_UNIT_EQ_BOOL(false, ah_bufc_read(&src, buffer, 1u));
        }
    }

    AH_UNIT_TEST("ah_bufc_read() copies from `c` to `dst`.")
    {
        const char* src_buffer = "A string of text.";
        size_t src_sz = strlen(src_buffer) + 1u;
        ah_bufc_t src = ah_bufc_from_readable(src_buffer, src_sz);

        uint8_t dst_buffer[32u];

        AH_UNIT_GE_UINT(sizeof(dst_buffer), src_sz);
        AH_UNIT_EQ_BOOL(true, ah_bufc_read(&src, dst_buffer, src_sz));
        AH_UNIT_EQ_UINT(0u, ah_bufc_get_readable_sz(&src));
        AH_UNIT_EQ_STR(src_buffer, (char*) dst_buffer);
    }

    AH_UNIT_TEST("ah_bufc_read_*() return 0u if `c` is NULL.")
    {
        AH_UNIT_EQ_UINT(0u, ah_bufc_read_u8(NULL));
        AH_UNIT_EQ_UINT(0u, ah_bufc_read_u16_be(NULL));
        AH_UNIT_EQ_UINT(0u, ah_bufc_read_u16_le(NULL));
        AH_UNIT_EQ_UINT(0u, ah_bufc_read_u32_be(NULL));
        AH_UNIT_EQ_UINT(0u, ah_bufc_read_u32_le(NULL));
        AH_UNIT_EQ_UINT(0u, ah_bufc_read_u64_be(NULL));
        AH_UNIT_EQ_UINT(0u, ah_bufc_read_u64_le(NULL));
    }

    AH_UNIT_TEST("ah_bufc_read_*() return 0u if `c` is close to end.")
    {
        uint8_t* buffer = (uint8_t*) "01234567";
        ah_bufc_t c;

        c = ah_bufc_from_readable(buffer, 0u);
        AH_UNIT_EQ_UINT(0u, ah_bufc_read_u8(&c));

        c = ah_bufc_from_readable(buffer, 1u);
        AH_UNIT_EQ_UINT(0u, ah_bufc_read_u16_be(&c));
        AH_UNIT_EQ_UINT(0u, ah_bufc_read_u16_le(&c));

        c = ah_bufc_from_readable(buffer, 3u);
        AH_UNIT_EQ_UINT(0u, ah_bufc_read_u32_be(&c));
        AH_UNIT_EQ_UINT(0u, ah_bufc_read_u32_le(&c));

        c = ah_bufc_from_readable(buffer, 7u);
        AH_UNIT_EQ_UINT(0u, ah_bufc_read_u64_be(&c));
        AH_UNIT_EQ_UINT(0u, ah_bufc_read_u64_le(&c));
    }

    AH_UNIT_TEST("ah_bufc_read_*() returns expected bytes.")
    {
        const char* buffer = "\x30\x31\x32\x33\x34\x35\x36\x37"
                             "\x38\x39\x3A\x3B\x3C\x3D\x3E\x3F";
        ah_bufc_t c;

        AH_UNIT_CASE("ah_bufc_read_u8()")
        {
            c = ah_bufc_from_readable(buffer, strlen(buffer));
            AH_UNIT_EQ_UHEX(0x30, ah_bufc_read_u8(&c));
            AH_UNIT_EQ_BOOL(true, ah_bufc_skip(&c, 2));
            AH_UNIT_EQ_UHEX(0x33, ah_bufc_read_u8(&c));
            AH_UNIT_EQ_BOOL(true, ah_bufc_skip(&c, 11));
            AH_UNIT_EQ_UHEX(0x3F, ah_bufc_read_u8(&c));
        }

        AH_UNIT_CASE("ah_bufc_read_u16_be()")
        {
            c = ah_bufc_from_readable(buffer, strlen(buffer));
            AH_UNIT_EQ_UHEX(ah_to_be_u16(0x3130), ah_bufc_read_u16_be(&c));
            AH_UNIT_EQ_BOOL(true, ah_bufc_skip(&c, 1));
            AH_UNIT_EQ_UHEX(ah_to_be_u16(0x3433), ah_bufc_read_u16_be(&c));
            AH_UNIT_EQ_BOOL(true, ah_bufc_skip(&c, 9));
            AH_UNIT_EQ_UHEX(ah_to_be_u16(0x3F3E), ah_bufc_read_u16_be(&c));
        }

        AH_UNIT_CASE("ah_bufc_read_u16_le()")
        {
            c = ah_bufc_from_readable(buffer, strlen(buffer));
            AH_UNIT_EQ_UHEX(ah_to_le_u16(0x3130), ah_bufc_read_u16_le(&c));
            AH_UNIT_EQ_BOOL(true, ah_bufc_skip(&c, 1));
            AH_UNIT_EQ_UHEX(ah_to_le_u16(0x3433), ah_bufc_read_u16_le(&c));
            AH_UNIT_EQ_BOOL(true, ah_bufc_skip(&c, 9));
            AH_UNIT_EQ_UHEX(ah_to_le_u16(0x3F3E), ah_bufc_read_u16_le(&c));
        }

        AH_UNIT_CASE("ah_bufc_read_u32_be()")
        {
            c = ah_bufc_from_readable(buffer, strlen(buffer));
            AH_UNIT_EQ_UHEX(ah_to_be_u32(0x33323130), ah_bufc_read_u32_be(&c));
            AH_UNIT_EQ_UHEX(ah_to_be_u32(0x37363534), ah_bufc_read_u32_be(&c));
            AH_UNIT_EQ_UHEX(ah_to_be_u32(0x3B3A3938), ah_bufc_read_u32_be(&c));
        }

        AH_UNIT_CASE("ah_bufc_read_u32_le()")
        {
            c = ah_bufc_from_readable(buffer, strlen(buffer));
            AH_UNIT_EQ_UHEX(ah_to_le_u32(0x33323130), ah_bufc_read_u32_le(&c));
            AH_UNIT_EQ_UHEX(ah_to_le_u32(0x37363534), ah_bufc_read_u32_le(&c));
            AH_UNIT_EQ_UHEX(ah_to_le_u32(0x3B3A3938), ah_bufc_read_u32_le(&c));
        }

        AH_UNIT_CASE("ah_bufc_read_u64_be()")
        {
            c = ah_bufc_from_readable(buffer, strlen(buffer));
            AH_UNIT_EQ_UHEX(ah_to_be_u64(0x3736353433323130), ah_bufc_read_u64_be(&c));
            AH_UNIT_EQ_UHEX(ah_to_be_u64(0x3F3E3D3C3B3A3938), ah_bufc_read_u64_be(&c));
        }

        AH_UNIT_CASE("ah_bufc_read_u64_le()")
        {
            c = ah_bufc_from_readable(buffer, strlen(buffer));
            AH_UNIT_EQ_UHEX(ah_to_le_u64(0x3736353433323130), ah_bufc_read_u64_le(&c));
            AH_UNIT_EQ_UHEX(ah_to_le_u64(0x3F3E3D3C3B3A3938), ah_bufc_read_u64_le(&c));
        }
    }
}
