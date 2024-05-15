// SPDX-License-Identifier: EPL-2.0

#include "ah/bit.h"
#include "ah/buf.h"

#include <ah/unit.h>

AH_UNIT_SUITE(bufc_write)
{
    AH_UNIT_TEST("ah_bufc_write() rejects invalid arguments.")
    {
        uint8_t buffer[32u];

        AH_UNIT_CASE("`c` is NULL.")
        {
            AH_UNIT_EQ_BOOL(false, ah_bufc_write(NULL, buffer, sizeof(buffer)));
        }

        AH_UNIT_CASE("`src` is NULL while `n` is non-zero.")
        {
            ah_bufc_t c = ah_bufc_from_writable(buffer, sizeof(buffer));
            AH_UNIT_EQ_BOOL(false, ah_bufc_write(&c, NULL, 1u));
        }

        AH_UNIT_CASE("`n` is too large for `c`.")
        {
            ah_bufc_t c = ah_bufc_from_writable(NULL, 0u);
            AH_UNIT_EQ_BOOL(false, ah_bufc_write(&c, (uint8_t*) "1234", 4u));
        }
    }

    AH_UNIT_TEST("ah_bufc_write() copies from `src` to `c`.")
    {
        uint8_t buffer[32u];
        ah_bufc_t c = ah_bufc_from_writable(buffer, sizeof(buffer));

        AH_UNIT_EQ_BOOL(true, ah_bufc_write(&c, (uint8_t*) "0123", 4u));
        AH_UNIT_EQ_BOOL(true, ah_bufc_write(&c, (uint8_t*) "4", 1u));
        AH_UNIT_EQ_BOOL(true, ah_bufc_write(&c, (uint8_t*) "56789ABCDEF", 11u));
        AH_UNIT_EQ_BOOL(true, ah_bufc_write(&c, (uint8_t*) "GHIJKLMNOP", 10u));
        AH_UNIT_EQ_BOOL(false, ah_bufc_write(&c, (uint8_t*) "QRSTUVWX", 8u));
        AH_UNIT_EQ_BOOL(true, ah_bufc_write(&c, (uint8_t*) "QRSTUV", 6u));
    }

    AH_UNIT_TEST("ah_bufc_write_u*() return false if `c` is NULL.")
    {
        AH_UNIT_EQ_BOOL(false, ah_bufc_write_u8(NULL, 1u));
        AH_UNIT_EQ_BOOL(false, ah_bufc_write_u16_be(NULL, 1u));
        AH_UNIT_EQ_BOOL(false, ah_bufc_write_u16_le(NULL, 1u));
        AH_UNIT_EQ_BOOL(false, ah_bufc_write_u32_be(NULL, 1u));
        AH_UNIT_EQ_BOOL(false, ah_bufc_write_u32_le(NULL, 1u));
        AH_UNIT_EQ_BOOL(false, ah_bufc_write_u64_be(NULL, 1u));
        AH_UNIT_EQ_BOOL(false, ah_bufc_write_u64_le(NULL, 1u));
    }

    AH_UNIT_TEST("ah_bufc_write_u*() return false if `c` is close to end.")
    {
        uint8_t buffer[32u];
        ah_bufc_t c;

        c = ah_bufc_from_writable(buffer, 0u);
        AH_UNIT_EQ_BOOL(false, ah_bufc_write_u8(&c, 1u));

        c = ah_bufc_from_writable(buffer, 1u);
        AH_UNIT_EQ_BOOL(false, ah_bufc_write_u16_be(&c, 2u));
        AH_UNIT_EQ_BOOL(false, ah_bufc_write_u16_le(&c, 3u));

        c = ah_bufc_from_writable(buffer, 3u);
        AH_UNIT_EQ_BOOL(false, ah_bufc_write_u32_be(&c, 4u));
        AH_UNIT_EQ_BOOL(false, ah_bufc_write_u32_le(&c, 5u));

        c = ah_bufc_from_writable(buffer, 7u);
        AH_UNIT_EQ_BOOL(false, ah_bufc_write_u64_be(&c, 6u));
        AH_UNIT_EQ_BOOL(false, ah_bufc_write_u64_le(&c, 7u));
    }

    AH_UNIT_TEST("ah_bufc_write_u*() writes words as expected.")
    {
        uint8_t buffer[16u];
        ah_bufc_t c;

        AH_UNIT_CASE("ah_bufc_write_u8()")
        {
            c = ah_bufc_from_writable(buffer, sizeof(buffer));

            AH_UNIT_EQ_BOOL(true, ah_bufc_write_u8(&c, 'H'));
            AH_UNIT_EQ_BOOL(true, ah_bufc_write_u8(&c, 'i'));
            AH_UNIT_EQ_BOOL(true, ah_bufc_write_u8(&c, '!'));
            AH_UNIT_EQ_BOOL(true, ah_bufc_write_u8(&c, '\0'));

            AH_UNIT_EQ_STR("Hi!", (const char*) buffer);
        }

        AH_UNIT_CASE("ah_bufc_write_u16_be()")
        {
            c = ah_bufc_from_writable(buffer, sizeof(buffer));

            AH_UNIT_EQ_BOOL(true, ah_bufc_write_u16_be(&c, 0x4865));
            AH_UNIT_EQ_BOOL(true, ah_bufc_write_u16_be(&c, 0x7921));
            AH_UNIT_EQ_BOOL(true, ah_bufc_write_u16_be(&c, 0x0000));

            AH_UNIT_EQ_STR("Hey!", (const char*) buffer);
        }

        AH_UNIT_CASE("ah_bufc_write_u16_le()")
        {
            c = ah_bufc_from_writable(buffer, sizeof(buffer));

            AH_UNIT_EQ_BOOL(true, ah_bufc_write_u16_le(&c, 0x6548));
            AH_UNIT_EQ_BOOL(true, ah_bufc_write_u16_le(&c, 0x2179));
            AH_UNIT_EQ_BOOL(true, ah_bufc_write_u16_le(&c, 0x0000));

            AH_UNIT_EQ_STR("Hey!", (const char*) buffer);
        }

        AH_UNIT_CASE("ah_bufc_write_u32_be()")
        {
            c = ah_bufc_from_writable(buffer, sizeof(buffer));

            AH_UNIT_EQ_BOOL(true, ah_bufc_write_u32_be(&c, 0x48656C6C));
            AH_UNIT_EQ_BOOL(true, ah_bufc_write_u32_be(&c, 0x6F2C2057));
            AH_UNIT_EQ_BOOL(true, ah_bufc_write_u32_be(&c, 0x6F726C64));
            AH_UNIT_EQ_BOOL(true, ah_bufc_write_u32_be(&c, 0x21000000));

            AH_UNIT_EQ_STR("Hello, World!", (const char*) buffer);
        }

        AH_UNIT_CASE("ah_bufc_write_u32_le()")
        {
            c = ah_bufc_from_writable(buffer, sizeof(buffer));

            AH_UNIT_EQ_BOOL(true, ah_bufc_write_u32_le(&c, 0x6C6C6548));
            AH_UNIT_EQ_BOOL(true, ah_bufc_write_u32_le(&c, 0x57202C6F));
            AH_UNIT_EQ_BOOL(true, ah_bufc_write_u32_le(&c, 0x646C726F));
            AH_UNIT_EQ_BOOL(true, ah_bufc_write_u32_le(&c, 0x00000021));

            AH_UNIT_EQ_STR("Hello, World!", (const char*) buffer);
        }

        AH_UNIT_CASE("ah_bufc_write_u64_be()")
        {
            c = ah_bufc_from_writable(buffer, sizeof(buffer));

            AH_UNIT_EQ_BOOL(true, ah_bufc_write_u64_be(&c, 0x48656C6C6F2C2043));
            AH_UNIT_EQ_BOOL(true, ah_bufc_write_u64_be(&c, 0x6974697A656E2100));

            AH_UNIT_EQ_STR("Hello, Citizen!", (const char*) buffer);
        }

        AH_UNIT_CASE("ah_bufc_write_u64_le()")
        {
            c = ah_bufc_from_writable(buffer, sizeof(buffer));

            AH_UNIT_EQ_BOOL(true, ah_bufc_write_u64_le(&c, 0x43202C6F6C6C6548));
            AH_UNIT_EQ_BOOL(true, ah_bufc_write_u64_le(&c, 0x00216E657A697469));

            AH_UNIT_EQ_STR("Hello, Citizen!", (const char*) buffer);
        }
    }

    AH_UNIT_TEST("ah_bufc_write_v() rejects invalid arguments.")
    {
        AH_UNIT_CASE("`c` is NULL.")
        {
            AH_UNIT_EQ_BOOL(false, ah_bufc_write_v(NULL, 0u));
        }

        AH_UNIT_CASE("`n` is too large for `c`.")
        {
            ah_bufc_t c = ah_bufc_from_writable(NULL, 0u);
            AH_UNIT_EQ_BOOL(false, ah_bufc_write_v(&c, 1u));
        }
    }

    AH_UNIT_TEST("ah_bufc_write_v() leaves \"written\" bytes unmodified.")
    {
        uint8_t buffer[] = { '1', '2', '3', '4', '\0' };
        ah_bufc_t c = ah_bufc_from_writable(buffer, sizeof(buffer));
        AH_UNIT_EQ_BOOL(true, ah_bufc_write_v(&c, 2));
        AH_UNIT_EQ_BOOL(true, ah_bufc_write_v(&c, 2));
        AH_UNIT_EQ_BOOL(true, ah_bufc_write_v(&c, 1));
        AH_UNIT_EQ_BOOL(false, ah_bufc_write_v(&c, 1));

        AH_UNIT_EQ_STR("1234", (const char*) buffer);
    }
}
