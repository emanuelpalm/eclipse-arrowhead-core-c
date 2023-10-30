// SPDX-License-Identifier: EPL-2.0

#ifndef AH_BIT_H
#define AH_BIT_H

#include "def.h"

#include <ahi/bit.h>

/**
 * @file
 * Special bitwise functions.
 *
 * Functions useful for inspecting and manipulating bit patterns.
 */

/**
 * Reverses the order of the bytes in @a u.
 *
 * @param u Integer to swap.
 *
 * @return Reversed variant of @a u.
 */
ah_inline uint16_t ah_byteswap_u16(uint16_t u) {
    return ahi_byteswap_u16(u);
}

/**
 * Reverses the order of the bytes in @a u.
 *
 * @param u Integer to swap.
 *
 * @return Reversed variant of @a u.
 */
ah_inline uint32_t ah_byteswap_u32(uint32_t u) {
    return ahi_byteswap_u32(u);
}

/**
 * Reverses the order of the bytes in @a u.
 *
 * @param u Integer to swap.
 *
 * @return Reversed variant of @a u.
 */
ah_inline uint64_t ah_byteswap_u64(uint64_t u) {
    return ahi_byteswap_u64(u);
}

/**
 * Converts @a u from big endian to platform byte order.
 *
 * @param u Integer to convert.
 *
 * @return Platform byte order variant of @a u.
 */
ah_inline uint16_t ah_from_be_u16(uint16_t u) {
    return ahi_from_be_u16(u);
}

/**
 * Converts @a u from big endian to platform byte order.
 *
 * @param u Integer to convert.
 *
 * @return Platform byte order variant of @a u.
 */
ah_inline uint16_t ah_from_be_u32(uint32_t u) {
    return ahi_from_be_u32(u);
}

/**
 * Converts @a u from big endian to platform byte order.
 *
 * @param u Integer to convert.
 *
 * @return Platform byte order variant of @a u.
 */
ah_inline uint16_t ah_from_be_u64(uint64_t u) {
    return ahi_from_be_u64(u);
}

/**
 * Converts @a u from little endian to platform byte order.
 *
 * @param u Integer to convert.
 *
 * @return Platform byte order variant of @a u.
 */
ah_inline uint16_t ah_from_le_u16(uint16_t u) {
    return ahi_from_le_u16(u);
}

/**
 * Converts @a u from little endian to platform byte order.
 *
 * @param u Integer to convert.
 *
 * @return Platform byte order variant of @a u.
 */
ah_inline uint16_t ah_from_le_u32(uint32_t u) {
    return ahi_from_le_u32(u);
}

/**
 * Converts @a u from little endian to platform byte order.
 *
 * @param u Integer to convert.
 *
 * @return Platform byte order variant of @a u.
 */
ah_inline uint16_t ah_from_le_u64(uint64_t u) {
    return ahi_from_le_u64(u);
}

/**
 * Converts @a u from platform to big endian byte order.
 *
 * @param u Integer to convert.
 *
 * @return Big endian byte order variant of @a u.
 */
ah_inline uint16_t ah_to_be_u16(uint16_t u) {
    return ahi_to_be_u16(u);
}

/**
 * Converts @a u from platform to big endian byte order.
 *
 * @param u Integer to convert.
 *
 * @return Big endian byte order variant of @a u.
 */
ah_inline uint16_t ah_to_be_u32(uint32_t u) {
    return ahi_to_be_u32(u);
}

/**
 * Converts @a u from platform to big endian byte order.
 *
 * @param u Integer to convert.
 *
 * @return Big endian byte order variant of @a u.
 */
ah_inline uint16_t ah_to_be_u64(uint64_t u) {
    return ahi_to_be_u64(u);
}

/**
 * Converts @a u from platform to little endian byte order.
 *
 * @param u Integer to convert.
 *
 * @return Little endian byte order variant of @a u.
 */
ah_inline uint16_t ah_to_le_u16(uint16_t u) {
    return ahi_to_le_u16(u);
}

/**
 * Converts @a u from platform to little endian byte order.
 *
 * @param u Integer to convert.
 *
 * @return Little endian byte order variant of @a u.
 */
ah_inline uint16_t ah_to_le_u32(uint32_t u) {
    return ahi_to_le_u32(u);
}

/**
 * Converts @a u from platform to little endian byte order.
 *
 * @param u Integer to convert.
 *
 * @return Little endian byte order variant of @a u.
 */
ah_inline uint16_t ah_to_le_u64(uint64_t u) {
    return ahi_to_le_u64(u);
}

#endif
