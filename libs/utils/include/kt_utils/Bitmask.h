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
///   template<> inline constexpr auto is_bitmask<MyEnum>{true};
/// \endcode
///
/// \tparam T scoped enum with `unsigned` underlying type and power of 2 values
template<typename T,
    std::enable_if_t<is_scoped_enum_v<T> &&
                         std::is_unsigned_v<std::underlying_type_t<T>>,
        int> = 0>
inline constexpr auto is_bitmask{false};

/// resolves to type `T` if #is_bitmask<T> has been specialized to true
/// \tparam T the scoped enum
/// \tparam _ placeholder for #is_bitmask second template arg
template<typename T, typename _ = T>
using isBitmask = std::enable_if_t<is_bitmask<T>, _>;

// the 7 bitwise operators enabled for 'bitmask' are: &, |, ^, ~, &=, |= and ^=

/// bitwise *and operator* for enum `T` enabled with #is_bitmask
template<typename T>
[[nodiscard]] constexpr isBitmask<T> operator&(T x, T y) noexcept {
  return to_enum<T>(to_underlying(x) & to_underlying(y));
}

/// bitwise *or operator* for enum `T` enabled with #is_bitmask
template<typename T>
[[nodiscard]] constexpr isBitmask<T> operator|(T x, T y) noexcept {
  return to_enum<T>(to_underlying(x) | to_underlying(y));
}

/// bitwise *xor operator* for enum `T` enabled with #is_bitmask
template<typename T>
[[nodiscard]] constexpr isBitmask<T> operator^(T x, T y) noexcept {
  return to_enum<T>(to_underlying(x) ^ to_underlying(y));
}

/// bitwise *complement operator* for enum `T` enabled with #is_bitmask
template<typename T>
[[nodiscard]] constexpr isBitmask<T> operator~(T x) noexcept {
  return to_enum<T>(~to_underlying(x));
}

/// bitwise *and-equal operator* for enum `T` enabled with #is_bitmask
template<typename T> constexpr isBitmask<T>& operator&=(T& x, T y) noexcept {
  return x = x & y;
}

/// bitwise *or-equal operator* for enum `T` enabled with #is_bitmask
template<typename T> constexpr isBitmask<T>& operator|=(T& x, T y) noexcept {
  return x = x | y;
}

/// bitwise *xor-equal operator* for enum `T` enabled with #is_bitmask
template<typename T> constexpr isBitmask<T>& operator^=(T& x, T y) noexcept {
  return x = x ^ y;
}

/// return true if `x` has a value, i.e., its underlying value is non-zero
/// \details can help in cases like `if (hasValue(myEnum & MyEnum::Flag1)) ...`
template<typename T>
[[nodiscard]] constexpr isBitmask<T, bool> hasValue(T x) noexcept {
  return to_underlying(x);
}

/// return true if `x` doesn't have a value, i.e., its underlying value is zero
/// \details can help in cases like `if (!(myEnum & MyEnum::Flag1)) ...`
template<typename T> [[nodiscard]] isBitmask<T, bool> operator!(T x) noexcept {
  return !hasValue(x);
}

/// \end_group
} // namespace kanji_tools
