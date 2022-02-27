#pragma once

#include <type_traits>

namespace kanji_tools {

// C++ 23 should include 'is_scoped_enum' and 'to_underlying' so the following
// code can be removed:

template<typename T, bool B = std::is_enum_v<T>>
struct is_scoped_enum : std::false_type {};

template<typename T>
struct is_scoped_enum<T, true>
    : std::integral_constant<
        bool, !std::is_convertible_v<T, std::underlying_type_t<T>>> {};

template<typename T>
inline constexpr bool is_scoped_enum_v = is_scoped_enum<T>::value;

template<typename T, std::enable_if_t<is_scoped_enum_v<T>, int> = 0>
[[nodiscard]] constexpr auto to_underlying(T x) noexcept {
  return static_cast<std::underlying_type_t<T>>(x);
}

} // namespace kanji_tools
