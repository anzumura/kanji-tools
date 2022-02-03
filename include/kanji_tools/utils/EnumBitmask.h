#ifndef KANJI_TOOLS_UTILS_ENUM_BITMASK_H
#define KANJI_TOOLS_UTILS_ENUM_BITMASK_H

#include <type_traits>

namespace kanji_tools {

// To meet the requirements for 'BitmaskType' (https://en.cppreference.com/w/cpp/named_req/BitmaskType)
// add a specialization of 'enum_bitmask' like the following:
//
// template<> struct enum_bitmask<MyEnum> { static constexpr bool value = true; };
//
// 'MyEnum' must be a scoped enum with values set to powers of 2. The specialization enables the 7
// required bitwise operators plus a 'hasValue' function (see EnumBitmaskTest.cpp for examples).

// C++ 23 should include 'is_scoped_enum' so the following code can be removed:
template<typename T, bool B = std::is_enum_v<T>> struct is_scoped_enum : std::false_type {};
template<typename T>
struct is_scoped_enum<T, true> : std::integral_constant<bool, !std::is_convertible_v<T, std::underlying_type_t<T>>> {};
template<typename T> inline constexpr bool is_scoped_enum_v = is_scoped_enum<T>::value;

// 'enum_bitmask' struct that needs to be specialized:
template<typename T, std::enable_if_t<is_scoped_enum_v<T>, int> = 0> struct enum_bitmask {
  static constexpr bool value = false;
};
template<typename T> inline constexpr bool enum_bitmask_v = enum_bitmask<T>::value;

// the 7 required bitwise operators: &, |, ^, ~, &=, |= and ^=
template<typename T, std::enable_if_t<enum_bitmask_v<T>, int> = 0> constexpr auto operator&(T x, T y) {
  return static_cast<T>(static_cast<std::underlying_type_t<T>>(x) & static_cast<std::underlying_type_t<T>>(y));
}
template<typename T, std::enable_if_t<enum_bitmask_v<T>, int> = 0> constexpr auto operator|(T x, T y) {
  return static_cast<T>(static_cast<std::underlying_type_t<T>>(x) | static_cast<std::underlying_type_t<T>>(y));
}
template<typename T, std::enable_if_t<enum_bitmask_v<T>, int> = 0> constexpr auto operator^(T x, T y) {
  return static_cast<T>(static_cast<std::underlying_type_t<T>>(x) ^ static_cast<std::underlying_type_t<T>>(y));
}
template<typename T, std::enable_if_t<enum_bitmask_v<T>, int> = 0> constexpr auto operator~(T x) {
  return static_cast<T>(~static_cast<std::underlying_type_t<T>>(x));
}
template<typename T, std::enable_if_t<enum_bitmask_v<T>, int> = 0> constexpr T& operator&=(T& x, T y) {
  x = x & y;
  return x;
}
template<typename T, std::enable_if_t<enum_bitmask_v<T>, int> = 0> constexpr T& operator|=(T& x, T y) {
  x = x | y;
  return x;
}
template<typename T, std::enable_if_t<enum_bitmask_v<T>, int> = 0> constexpr T& operator^=(T& x, T y) {
  x = x ^ y;
  return x;
}

// 'hasValue' can help in cases like 'if (hasValue(myEnum & MyEnum::Flag1)) ...'
template<typename T, std::enable_if_t<enum_bitmask_v<T>, int> = 0> constexpr auto hasValue(T x) {
  return static_cast<std::underlying_type_t<T>>(x) != 0;
}

} // namespace kanji_tools

#endif // KANJI_TOOLS_UTILS_ENUM_BITMASK_H
