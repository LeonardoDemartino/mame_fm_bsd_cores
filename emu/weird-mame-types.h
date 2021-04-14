#pragma once
#include <stdint.h>

extern int mute_mask;
extern int channelsOutput[64];

#define u8  uint8_t
#define s8   int8_t
#define u16 uint16_t
#define s16  int16_t
#define u32 uint32_t

#define s32  int32_t

#ifdef __cplusplus
#include <vector>
using offs_t = unsigned int;

inline uint8_t count_leading_zeros(uint32_t val)
{
	if (!val) return 32U;
	uint8_t count;
	for (count = 0; int32_t(val) >= 0; count++) val <<= 1;
	return count;
}

template <typename T, typename U> constexpr T make_bitmask(U n)
{
	return T((n < (8 * sizeof(T)) ? (std::make_unsigned_t<T>(1) << n) : std::make_unsigned_t<T>(0)) - 1);
}

template <typename T, typename U> constexpr T BIT(T x, U n) noexcept { return (x >> n) & T(1); }

template <typename T, typename U, typename V> constexpr T BIT(T x, U n, V w)
{
	return (x >> n) & make_bitmask<T>(w);
}

template <typename T, typename U> constexpr T bitswap(T val, U b) noexcept { return BIT(val, b) << 0U; }

template <typename T, typename U, typename... V> constexpr T bitswap(T val, U b, V... c) noexcept
{
	return (BIT(val, b) << sizeof...(c)) | bitswap(val, c...);
}

template <unsigned B, typename T, typename... U> T bitswap(T val, U... b) noexcept
{
	static_assert(sizeof...(b) == B, "wrong number of bits");
	static_assert((sizeof(std::remove_reference_t<T>) * 8) >= B, "return type too small for result");
	return bitswap(val, b...);
}
#endif
