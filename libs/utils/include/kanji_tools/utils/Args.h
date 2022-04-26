#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

namespace kanji_tools {

// helper class to hold command line args
class Args {
public:
  using Size = u_int16_t;
  using List = const char* const*;

  Args() noexcept = default;
  Args(Size size, List args);
  // provide an 'int' overload that will throw if 'size' is out of range
  Args(int size, List args) : Args{checkInt(size), args} {}

  // 'const char*[]' ctor is helpful for test code since it figures out 'size'
  // from the array, it's also not marked 'explicit' to help shorten code.
  template<size_t N>
  constexpr Args(const char* (&args)[N]) noexcept // NOLINT
      : _size{N}, _args{args} {
    static_assert(N <= std::numeric_limits<Size>::max());
  }

  [[nodiscard]] const char* operator[](Size) const;
  [[nodiscard]] constexpr auto size() const { return _size; }
  [[nodiscard]] constexpr explicit operator bool() const { return _size; }
private:
  [[nodiscard]] static Size checkInt(int);

  const Size _size{};
  List _args{};
};

} // namespace kanji_tools
