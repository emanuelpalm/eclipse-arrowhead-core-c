// SPDX-License-Identifier: EPL-2.0

#ifndef AHI_BIT_H
#define AHI_BIT_H

#define ahi_byteswap_u16(v) __builtin_bswap16(v)
#define ahi_byteswap_u32(v) __builtin_bswap32(v)
#define ahi_byteswap_u64(v) __builtin_bswap64(v)

#ifdef __BYTE_ORDER__
#define AHI_ENDIAN_BIG    __ORDER_BIG_ENDIAN__
#define AHI_ENDIAN_LITTLE __ORDER_LITTLE_ENDIAN__
#define AHI_ENDIAN_NATIVE __BYTE_ORDER__
#else
# error "Unable to determine platform byte order."
#endif

#if AHI_ENDIAN_NATIVE == AHI_ENDIAN_BIG
# define ahi_from_be_u16(v) (v)
# define ahi_from_be_u32(v) (v)
# define ahi_from_be_u64(v) (v)
# define ahi_from_le_u16(v) ahi_byteswap_u16(v)
# define ahi_from_le_u32(v) ahi_byteswap_u32(v)
# define ahi_from_le_u64(v) ahi_byteswap_u64(v)
# define ahi_to_be_u16(v) (v)
# define ahi_to_be_u32(v) (v)
# define ahi_to_be_u64(v) (v)
# define ahi_to_le_u16(v) ahi_byteswap_u16(v)
# define ahi_to_le_u32(v) ahi_byteswap_u32(v)
# define ahi_to_le_u64(v) ahi_byteswap_u64(v)
#elif AHI_ENDIAN_NATIVE == AHI_ENDIAN_LITTLE
# define ahi_from_be_u16(v) ahi_byteswap_u16(v)
# define ahi_from_be_u32(v) ahi_byteswap_u32(v)
# define ahi_from_be_u64(v) ahi_byteswap_u64(v)
# define ahi_from_le_u16(v) (v)
# define ahi_from_le_u32(v) (v)
# define ahi_from_le_u64(v) (v)
# define ahi_to_be_u16(v) ahi_byteswap_u16(v)
# define ahi_to_be_u32(v) ahi_byteswap_u32(v)
# define ahi_to_be_u64(v) ahi_byteswap_u64(v)
# define ahi_to_le_u16(v) (v)
# define ahi_to_le_u32(v) (v)
# define ahi_to_le_u64(v) (v)
#else
# error "Unsupported platform byte order."
#endif

#endif
