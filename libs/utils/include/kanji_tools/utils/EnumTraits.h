#pragma once

#include <type_traits>

namespace kanji_tools {

// C++ 23 should include 'is_scoped_enum' and 'to_underlying' so the following
// code can be removed:

template<typename T, bool B = std::is_enum_v<T>>
struct is_scoped_enum : std::false_type {};

template<typename T>
struct is_scoped_enum<T, true>
    : std::integral_constant<bool,
          !std::is_convertible_v<T, std::underlying_type_t<T>>> {};

template<typename T>
inline constexpr auto is_scoped_enum_v{is_scoped_enum<T>::value};

template<typename T, std::enable_if_t<is_scoped_enum_v<T>, int> = 0>
[[nodiscard]] constexpr auto to_underlying(T x) noexcept {
  return static_cast<std::underlying_type_t<T>>(x);
}

// cast from a scoped enum's underlying type back to the enum - this is safer
// than a plain static_cast since T must be a scoped enum and the function param
// is of the corresponding underlying type for T
template<typename T>
[[nodiscard]] constexpr std::enable_if_t<is_scoped_enum_v<T>, T> to_enum(
    std::underlying_type_t<T> u) noexcept {
  return static_cast<T>(u);
}

} // namespace kanji_tools
