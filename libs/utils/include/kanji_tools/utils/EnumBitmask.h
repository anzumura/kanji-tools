#pragma once

#include <kanji_tools/utils/EnumTraits.h>

namespace kanji_tools {

// To enable operators to meet the requirements of 'BitmaskType', specialize
// 'is_bitmask' for a scoped enum like the following:
//
//   template<> inline constexpr auto is_bitmask<MyEnum>{true};
//
// See https://en.cppreference.com/w/cpp/named_req/BitmaskType for details.
//
// 'MyEnum' must be a scoped enum with values set to powers of 2. The
// specialization enables the 7 required bitwise operators plus 'hasValue'
// and 'operator!' global functions (see EnumBitmaskTest.cpp for examples).

// 'is_bitmask' bool that should be specialized:
template<typename T, std::enable_if_t<is_scoped_enum_v<T>, int> = 0>
inline constexpr auto is_bitmask{false};

template<typename T, typename _ = T>
using isBitmask = std::enable_if_t<is_bitmask<T>, _>;

// the 7 required bitwise operators are: &, |, ^, ~, &=, |= and ^=

template<typename T>
[[nodiscard]] constexpr isBitmask<T> operator&(T x, T y) noexcept {
  return static_cast<T>(to_underlying(x) & to_underlying(y));
}

template<typename T>
[[nodiscard]] constexpr isBitmask<T> operator|(T x, T y) noexcept {
  return static_cast<T>(to_underlying(x) | to_underlying(y));
}

template<typename T>
[[nodiscard]] constexpr isBitmask<T> operator^(T x, T y) noexcept {
  return static_cast<T>(to_underlying(x) ^ to_underlying(y));
}

template<typename T>
[[nodiscard]] constexpr isBitmask<T> operator~(T x) noexcept {
  return static_cast<T>(~to_underlying(x));
}

template<typename T> constexpr isBitmask<T>& operator&=(T& x, T y) noexcept {
  return x = x & y;
}

template<typename T> constexpr isBitmask<T>& operator|=(T& x, T y) noexcept {
  return x = x | y;
}

template<typename T> constexpr isBitmask<T>& operator^=(T& x, T y) noexcept {
  return x = x ^ y;
}

// 'hasValue' can help in cases like 'if (hasValue(myEnum & MyEnum::Flag1)) ...'
template<typename T>
[[nodiscard]] constexpr isBitmask<T, bool> hasValue(T x) noexcept {
  return to_underlying(x);
}

// 'operator!' can help in cases like 'if (!(myEnum & MyEnum::Flag1)) ...'
template<typename T> [[nodiscard]] isBitmask<T, bool> operator!(T x) noexcept {
  return !hasValue(x);
}

} // namespace kanji_tools