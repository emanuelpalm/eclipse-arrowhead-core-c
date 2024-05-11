// SPDX-License-Identifier: EPL-2.0

#include "ah/bit.h"

#include <ah/unit.h>

AH_UNIT_SUITE(bit)
{
    AH_UNIT_TEST("ah_byteswap_u16() swaps as expected.")
    {
        struct ah_Case {
            uint16_t before;
            uint16_t after;
        };

        static const struct ah_Case cases[] = {
            { 0x0000, 0x0000 },
            { 0x0001, 0x0100 },
            { 0x00FF, 0xFF00 },
            { 0x8421, 0x2184 },
        };

        for (size_t i = 0u; i < sizeof(cases) / sizeof(struct ah_Case); i++) {
            struct ah_Case c = cases[i];

            AH_UNIT_CASE("before: %p, after: %p", c.before, c.after)
            {
                uint16_t res = ah_byteswap_u16(c.before);
                AH_UNIT_EQ_UHEX(c.after, res);
            }
        }
    }

    AH_UNIT_TEST("ah_byteswap_u32() swaps as expected.")
    {
        struct ah_Case {
            uint32_t before;
            uint32_t after;
        };

        static const struct ah_Case cases[] = {
            { 0x00000000, 0x00000000 },
            { 0x00000001, 0x01000000 },
            { 0x0000FFFF, 0xFFFF0000 },
            { 0x87654321, 0x21436587 },
        };

        for (size_t i = 0u; i < sizeof(cases) / sizeof(struct ah_Case); i++) {
            struct ah_Case c = cases[i];

            AH_UNIT_CASE("before: %p, after: %p", c.before, c.after)
            {
                uint32_t res = ah_byteswap_u32(c.before);
                AH_UNIT_EQ_UHEX(c.after, res);
            }
        }
    }

    AH_UNIT_TEST("ah_byteswap_u64() swaps as expected.")
    {
        struct ah_Case {
            uint64_t before;
            uint64_t after;
        };

        static const struct ah_Case cases[] = {
            { 0x0000000000000000, 0x0000000000000000 },
            { 0x0000000000000001, 0x0100000000000000 },
            { 0x00000000FFFFFFFF, 0xFFFFFFFF00000000 },
            { 0xFEDCBA9876543210, 0x1032547698BADCFE },
        };

        for (size_t i = 0u; i < sizeof(cases) / sizeof(struct ah_Case); i++) {
            struct ah_Case c = cases[i];

            AH_UNIT_CASE("before: %p, after: %p", c.before, c.after)
            {
                uint64_t res = ah_byteswap_u64(c.before);
                AH_UNIT_EQ_UHEX(c.after, res);
            }
        }
    }

    AH_UNIT_TEST("Convert integers from big endian")
    {
#if AH_ENDIAN_NATIVE == AH_ENDIAN_BIG
        AH_UNIT_EQ_UHEX(0x8421, ah_from_be_u16(0x8421));
        AH_UNIT_EQ_UHEX(0x87654321, ah_from_be_u32(0x87654321));
        AH_UNIT_EQ_UHEX(0xFEDCBA9876543210, ah_from_be_u64(0xFEDCBA9876543210));
#elif AH_ENDIAN_NATIVE == AH_ENDIAN_LITTLE
        AH_UNIT_EQ_UHEX(0x2184, ah_from_be_u16(0x8421));
        AH_UNIT_EQ_UHEX(0x21436587, ah_from_be_u32(0x87654321));
        AH_UNIT_EQ_UHEX(0x1032547698BADCFE, ah_from_be_u64(0xFEDCBA9876543210));
#endif
    }

    AH_UNIT_TEST("Convert integers from little endian")
    {
#if AH_ENDIAN_NATIVE == AH_ENDIAN_BIG
        AH_UNIT_EQ_UHEX(0x2184, ah_from_le_u16(0x8421));
        AH_UNIT_EQ_UHEX(0x21436587, ah_from_le_u32(0x87654321));
        AH_UNIT_EQ_UHEX(0x1032547698BADCFE, ah_from_le_u64(0xFEDCBA9876543210));
#elif AH_ENDIAN_NATIVE == AH_ENDIAN_LITTLE
        AH_UNIT_EQ_UHEX(0x8421, ah_from_le_u16(0x8421));
        AH_UNIT_EQ_UHEX(0x87654321, ah_from_le_u32(0x87654321));
        AH_UNIT_EQ_UHEX(0xFEDCBA9876543210, ah_from_le_u64(0xFEDCBA9876543210));
#endif
    }

    AH_UNIT_TEST("Convert integers to big endian")
    {
#if AH_ENDIAN_NATIVE == AH_ENDIAN_BIG
        AH_UNIT_EQ_UHEX(0x8421, ah_to_be_u16(0x8421));
        AH_UNIT_EQ_UHEX(0x87654321, ah_to_be_u32(0x87654321));
        AH_UNIT_EQ_UHEX(0xFEDCBA9876543210, ah_to_be_u64(0xFEDCBA9876543210));
#elif AH_ENDIAN_NATIVE == AH_ENDIAN_LITTLE
        AH_UNIT_EQ_UHEX(0x2184, ah_to_be_u16(0x8421));
        AH_UNIT_EQ_UHEX(0x21436587, ah_to_be_u32(0x87654321));
        AH_UNIT_EQ_UHEX(0x1032547698BADCFE, ah_to_be_u64(0xFEDCBA9876543210));
#endif
    }

    AH_UNIT_TEST("Convert integers to little endian")
    {
#if AH_ENDIAN_NATIVE == AH_ENDIAN_BIG
        AH_UNIT_EQ_UHEX(0x2184, ah_to_le_u16(0x8421));
        AH_UNIT_EQ_UHEX(0x21436587, ah_to_le_u32(0x87654321));
        AH_UNIT_EQ_UHEX(0x1032547698BADCFE, ah_to_le_u64(0xFEDCBA9876543210));
#elif AH_ENDIAN_NATIVE == AH_ENDIAN_LITTLE
        AH_UNIT_EQ_UHEX(0x8421, ah_to_le_u16(0x8421));
        AH_UNIT_EQ_UHEX(0x87654321, ah_to_le_u32(0x87654321));
        AH_UNIT_EQ_UHEX(0xFEDCBA9876543210, ah_to_le_u64(0xFEDCBA9876543210));
#endif
    }
}
