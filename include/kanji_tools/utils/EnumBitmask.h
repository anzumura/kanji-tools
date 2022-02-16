#ifndef KANJI_TOOLS_UTILS_ENUM_BITMASK_H
#define KANJI_TOOLS_UTILS_ENUM_BITMASK_H

#include <kanji_tools/utils/EnumTraits.h>

namespace kanji_tools {

// To meet the requirements for 'BitmaskType' (https://en.cppreference.com/w/cpp/named_req/BitmaskType)
// add a specialization of 'is_bitmask' like the following:
//
// template<> inline constexpr bool is_bitmask<MyEnum> = true;
//
// 'MyEnum' must be a scoped enum with values set to powers of 2. The specialization enables the 7
// required bitwise operators plus a 'hasValue' function (see EnumBitmaskTest.cpp for examples).

// 'is_bitmask' bool that should be specialized:
template<typename T, std::enable_if_t<is_scoped_enum_v<T>, int> = 0> inline constexpr bool is_bitmask = false;

// the 7 required bitwise operators: &, |, ^, ~, &=, |= and ^=
template<typename T> [[nodiscard]] constexpr std::enable_if_t<is_bitmask<T>, T> operator&(T x, T y) noexcept {
  return static_cast<T>(to_underlying(x) & to_underlying(y));
}
template<typename T> [[nodiscard]] constexpr std::enable_if_t<is_bitmask<T>, T> operator|(T x, T y) noexcept {
  return static_cast<T>(to_underlying(x) | to_underlying(y));
}
template<typename T> [[nodiscard]] constexpr std::enable_if_t<is_bitmask<T>, T> operator^(T x, T y) noexcept {
  return static_cast<T>(to_underlying(x) ^ to_underlying(y));
}
template<typename T> [[nodiscard]] constexpr std::enable_if_t<is_bitmask<T>, T> operator~(T x) noexcept {
  return static_cast<T>(~to_underlying(x));
}
template<typename T> constexpr std::enable_if_t<is_bitmask<T>, T&> operator&=(T& x, T y) noexcept { return x = x & y; }
template<typename T> constexpr std::enable_if_t<is_bitmask<T>, T&> operator|=(T& x, T y) noexcept { return x = x | y; }
template<typename T> constexpr std::enable_if_t<is_bitmask<T>, T&> operator^=(T& x, T y) noexcept { return x = x ^ y; }

// 'hasValue' can help in cases like 'if (hasValue(myEnum & MyEnum::Flag1)) ...'
template<typename T> [[nodiscard]] constexpr std::enable_if_t<is_bitmask<T>, bool> hasValue(T x) noexcept {
  return to_underlying(x);
}

// 'operator!' can help in cases like 'if (!(myEnum & MyEnum::Flag1)) ...'
template<typename T> [[nodiscard]] constexpr std::enable_if_t<is_bitmask<T>, bool> operator!(T x) noexcept {
  return !hasValue(x);
}

} // namespace kanji_tools

#endif // KANJI_TOOLS_UTILS_ENUM_BITMASK_H
