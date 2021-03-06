#pragma once

#include <type_traits>

namespace kanji_tools { /// \utils_group{TypeTraits}
/// structs and functions for working with types (some of which are part of the
/// upcoming 'C++23' standard)

/// test if `T` is a scoped enum (part of 'C++23') \utils{TypeTraits}
template <typename T, bool B = std::is_enum_v<T>>
struct is_scoped_enum final : std::false_type {};

/// `value` is true if `T` is a scoped enum (part of 'C++23') \utils{TypeTraits}
template <typename T>
struct is_scoped_enum<T, true> final
    : std::integral_constant<bool,
          !std::is_convertible_v<T, std::underlying_type_t<T>>> {};

/// true if `T` is a scoped enum (part of 'C++23')
template <typename T> constexpr auto is_scoped_enum_v{is_scoped_enum<T>::value};

/// `T` is a scoped enum
template <typename T>
concept scoped_enum = is_scoped_enum_v<T>;

/// `T` is a scoped enum with an `unsigned` underlying type
template <typename T>
concept unsigned_scoped_enum =
    scoped_enum<T> && std::is_unsigned_v<std::underlying_type_t<T>>;

/// return the underlying value of `x` (part of 'C++23')
template <scoped_enum T>
[[nodiscard]] constexpr auto to_underlying(T x) noexcept {
  return static_cast<std::underlying_type_t<T>>(x);
}

/// cast `u` to a scoped enum - safer than using `static_cast` since `T` must be
/// a scoped enum and `u` is underlying type for `T`
template <scoped_enum T>
[[nodiscard]] constexpr T to_enum(std::underlying_type_t<T> u) noexcept {
  return static_cast<T>(u);
}

/// \end_group
} // namespace kanji_tools
