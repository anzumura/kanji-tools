#pragma once

#include <kt_utils/Exception.h>
#include <kt_utils/String.h>

#include <iostream>
#include <optional>

namespace kanji_tools {

/// find `expected` lines in `is`
/// \tparam P predicate function for checking for a match
/// \tparam T type of `expected`
/// \param is input stream to search
/// \param expected list of values to look for in `is`
/// \return first 'not found' value or `nullopt` if all values were found
template <auto P, typename T>
[[nodiscard]] auto findMatches(std::istream& is, const T& expected)
    -> std::optional<std::remove_cvref_t<decltype(*std::begin(expected))>> {
  auto i{std::begin(expected)};
  const auto end{std::end(expected)};
  if (i == end) throw DomainError{"expected cannot be empty"};
  for (String line; std::getline(is, line);)
    if (P(line, *i) && ++i == end) return {};
  return std::optional(*i);
}

/// find lines equal to `expected` values in `is`
/// \tparam T type of `expected`
/// \param is input stream to search
/// \param expected list of values to look for in `is`
/// \return first 'not found' value or `nullopt` if all values were found
template <typename T>
[[nodiscard]] auto findEqualMatches(std::istream& is, const T& expected) {
  return findMatches<[](const String& x, const auto& y) { return x == y; }>(
      is, expected);
}

/// find lines ending with `expected` values in `is`
/// \tparam T type of `expected`
/// \param is input stream to search
/// \param expected list of values to look for in `is`
/// \return first 'not found' value or `nullopt` if all values were found
template <typename T>
[[nodiscard]] auto findEndMatches(std::istream& is, const T& expected) {
  return findMatches<[](const String& x, const auto& y) {
    return x.ends_with(y);
  }>(is, expected);
}

[[nodiscard]] inline auto hasMoreData(std::istream& is) {
  return is.peek() != std::ios::traits_type::eof();
}

} // namespace kanji_tools
