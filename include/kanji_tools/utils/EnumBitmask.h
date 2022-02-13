#ifndef KANJI_TOOLS_UTILS_ENUM_BITMASK_H
#define KANJI_TOOLS_UTILS_ENUM_BITMASK_H

#include <type_traits>

namespace kanji_tools {

// To meet the requirements for 'BitmaskType' (https://en.cppreference.com/w/cpp/named_req/BitmaskType)
// add a specialization of 'is_bitmask' like the following:
//
// template<> inline constexpr bool is_bitmask<MyEnum> = true;
//
// 'MyEnum' must be a scoped enum with values set to powers of 2. The specialization enables the 7
// required bitwise operators plus a 'hasValue' function (see EnumBitmaskTest.cpp for examples).

// C++ 23 should include 'is_scoped_enum' and 'to_underlying' so the following code can be removed:
template<typename T, bool B = std::is_enum_v<T>> struct is_scoped_enum : std::false_type {};
template<typename T>
struct is_scoped_enum<T, true> : std::integral_constant<bool, !std::is_convertible_v<T, std::underlying_type_t<T>>> {};
template<typename T> inline constexpr bool is_scoped_enum_v = is_scoped_enum<T>::value;
template<typename T, std::enable_if_t<is_scoped_enum_v<T>, int> = 0>
[[nodiscard]] constexpr auto to_underlying(T x) noexcept {
  return static_cast<std::underlying_type_t<T>>(x);
}

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

} // namespace kanji_tools

#endif // KANJI_TOOLS_UTILS_ENUM_BITMASK_H
