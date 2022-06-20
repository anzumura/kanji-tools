#pragma once

#include <kt_utils/TypeTraits.h>

namespace kanji_tools { /// \utils_group{Bitmask}
/// enable using bitwise operators with a scoped enum

/// specialize to enable the 7 bitwise operators that satisfy C++ 'BitmaskType'
/// requirements as well as hasValue() and operator!() global functions (see
/// BitmaskTest.cpp for examples)
/// \sa https://en.cppreference.com/w/cpp/named_req/BitmaskType
///
/// Example code for enabling this functionality for 'MyEnum':
/// \code
///   template <> inline constexpr auto is_bitmask<MyEnum>{true};
/// \endcode
///
/// \tparam T scoped enum with `unsigned` underlying type and power of 2 values
template <unsigned_scoped_enum T> constexpr auto is_bitmask{false};

template <typename T>
concept bitmask = is_bitmask<T>;

// the 7 bitwise operators enabled for 'bitmask' are: &, |, ^, ~, &=, |= and ^=

/// 'bitwise and operator' for enum `T` enabled with #is_bitmask
template <bitmask T> [[nodiscard]] constexpr T operator&(T x, T y) noexcept {
  return to_enum<T>(to_underlying(x) & to_underlying(y));
}

/// 'bitwise or operator' for enum `T` enabled with #is_bitmask
template <bitmask T> [[nodiscard]] constexpr T operator|(T x, T y) noexcept {
  return to_enum<T>(to_underlying(x) | to_underlying(y));
}

/// 'bitwise xor operator' for enum `T` enabled with #is_bitmask
template <bitmask T> [[nodiscard]] constexpr T operator^(T x, T y) noexcept {
  return to_enum<T>(to_underlying(x) ^ to_underlying(y));
}

/// 'bitwise complement operator' for enum `T` enabled with #is_bitmask
template <bitmask T> [[nodiscard]] constexpr T operator~(T x) noexcept {
  return to_enum<T>(~to_underlying(x));
}

/// 'bitwise and-equal operator' for enum `T` enabled with #is_bitmask
template <bitmask T> constexpr T& operator&=(T& x, T y) noexcept {
  return x = x & y;
}

/// 'bitwise or-equal operator' for enum `T` enabled with #is_bitmask
template <bitmask T> constexpr T& operator|=(T& x, T y) noexcept {
  return x = x | y;
}

/// 'bitwise xor-equal operator' for enum `T` enabled with #is_bitmask
template <bitmask T> constexpr T& operator^=(T& x, T y) noexcept {
  return x = x ^ y;
}

/// return true if `x` has a value, i.e., its underlying value is non-zero
/// \details can help in cases like `if (hasValue(myEnum & MyEnum::Flag1)) ...`
template <bitmask T> [[nodiscard]] constexpr bool hasValue(T x) noexcept {
  return to_underlying(x);
}

/// return true if `x` doesn't have a value, i.e., its underlying value is zero
/// \details can help in cases like `if (!(myEnum & MyEnum::Flag1)) ...`
template <bitmask T> [[nodiscard]] constexpr bool operator!(T x) noexcept {
  return !hasValue(x);
}

/// \end_group
} // namespace kanji_tools
