#pragma once

#include <kt_utils/String.h>

#include <iostream>

namespace kanji_tools {

/// count number of lines in `is` that match an entry in `expected`
/// \tparam P predicate function for checking for a match
/// \tparam T type of `expected`
/// \param is input stream to search
/// \param expected list of values to look for in `is`
/// \return total number of matching lines found
template<auto P, typename T>
[[nodiscard]] auto findMatches(std::istream& is, const T& expected) {
  size_t found{};
  for (String line; std::getline(is, line);)
    for (const auto& i : expected)
      if (P(line, i)) {
        ++found;
        break;
      }
  return found;
}

/// count number of lines in `is` that equal an entry in `expected`
/// \tparam T type of `expected`
/// \param is input stream to search
/// \param expected list of values to look for in `is`
/// \return total number of matching lines found
template<typename T>
[[nodiscard]] auto findEqualMatches(std::istream& is, const T& expected) {
  return findMatches<[](const String& x, const auto& y) { return x == y; }>(
      is, expected);
}

/// count number of lines in `is` that end with an entry in `expected`
/// \tparam T type of `expected`
/// \param is input stream to search
/// \param expected list of values to look for in `is`
/// \return total number of matching lines found
template<typename T>
[[nodiscard]] auto findEndMatches(std::istream& is, const T& expected) {
  return findMatches<[](const String& x, const auto& y) {
    return x.ends_with(y);
  }>(is, expected);
}

} // namespace kanji_tools
