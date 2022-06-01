#pragma once

#include <type_traits>

namespace kanji_tools { /// \utils_group{TypeTraits}
/// structs and functions for working with types (some of which are part of the
/// upcoming 'C++23' standard)

/// struct for testing if `T` is a scoped enum (will be included in 'C++23')
template<typename T, bool B = std::is_enum_v<T>>
struct is_scoped_enum : std::false_type {};

/// `value` member is true if `T` is a scoped enum (will be included 'C++ 23')
template<typename T>
struct is_scoped_enum<T, true>
    : std::integral_constant<bool,
          !std::is_convertible_v<T, std::underlying_type_t<T>>> {};

/// true if `T` is a scoped enum (will be included 'C++ 23')
template<typename T>
inline constexpr auto is_scoped_enum_v{is_scoped_enum<T>::value};

/// return the underlying value of `x` (will be included 'C++ 23')
template<typename T, std::enable_if_t<is_scoped_enum_v<T>, int> = 0>
[[nodiscard]] constexpr auto to_underlying(T x) noexcept {
  return static_cast<std::underlying_type_t<T>>(x);
}

/// cast `u` to a scoped enum - this is safer than using `static_cast` since `T`
/// must be a scoped enum and `u` has the actual underlying type for `T`
template<typename T>
[[nodiscard]] constexpr std::enable_if_t<is_scoped_enum_v<T>, T> to_enum(
    std::underlying_type_t<T> u) noexcept {
  return static_cast<T>(u);
}

/// \end_group
} // namespace kanji_tools
