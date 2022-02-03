#ifndef KANJI_TOOLS_UTILS_ENUM_BITMASK_H
#define KANJI_TOOLS_UTILS_ENUM_BITMASK_H

#include <type_traits>

namespace kanji_tools {

// C++ 23 will have 'is_scoped_enum' so the following code can be removed
template<typename T, bool B = std::is_enum_v<T>> struct is_scoped_enum : std::false_type {};
template<typename T>
struct is_scoped_enum<T, true> : std::integral_constant<bool, !std::is_convertible_v<T, std::underlying_type_t<T>>> {};
template<typename T> inline constexpr bool is_scoped_enum_v = is_scoped_enum<T>::value;

// add a specialization of 'enum_bitmask' like the following to enable bitwise operators for a scoped enum:
// template<> struct enum_bitmask<MyEnum> { static constexpr bool value = true; };
template<typename T, std::enable_if_t<is_scoped_enum_v<T>, int> = 0> struct enum_bitmask {
  static constexpr bool value = false;
};
template<typename T> inline constexpr bool enum_bitmask_v = enum_bitmask<T>::value;

template<typename T, std::enable_if_t<enum_bitmask_v<T>, int> = 0> constexpr T operator&(T x, T y) {
  return static_cast<T>(static_cast<std::underlying_type_t<T>>(x) & static_cast<std::underlying_type_t<T>>(y));
}

template<typename T, std::enable_if_t<enum_bitmask_v<T>, int> = 0> constexpr T operator|(T x, T y) {
  return static_cast<T>(static_cast<std::underlying_type_t<T>>(x) | static_cast<std::underlying_type_t<T>>(y));
}

template<typename T, std::enable_if_t<enum_bitmask_v<T>, int> = 0> constexpr T operator^(T x, T y) {
  return static_cast<T>(static_cast<std::underlying_type_t<T>>(x) ^ static_cast<std::underlying_type_t<T>>(y));
}

template<typename T, std::enable_if_t<enum_bitmask_v<T>, int> = 0> constexpr T operator~(T x) {
  return static_cast<T>(~static_cast<std::underlying_type_t<T>>(x));
}

template<typename T> constexpr T& operator&=(T& x, T y) {
  x = x & y;
  return x;
}

template<typename T> constexpr T& operator|=(T& x, T y) {
  x = x | y;
  return x;
}

template<typename T> constexpr T& operator^=(T& x, T y) {
  x = x ^ y;
  return x;
}

// 'hasValue' can help in cases like 'if (hasValue(myEnum & MyEnum::Flag1)) ...'
template<typename T, std::enable_if_t<enum_bitmask_v<T>, int> = 0> constexpr bool hasValue(T x) {
  return static_cast<std::underlying_type_t<T>>(x) != 0;
}

} // namespace kanji_tools

#endif // KANJI_TOOLS_UTILS_ENUM_BITMASK_H
