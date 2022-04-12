#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

namespace kanji_tools {

// helper class to hold command line args
class Args {
public:
  using Size = u_int8_t;
  using List = const char* const*;

  Args() noexcept = default;
  template<size_t N>
  Args(const char* (&args)[N]) noexcept : _size{N}, _args{args} {
    static_assert(N <= std::numeric_limits<Size>::max());
  }
  Args(Size size, List args);
  // provide an 'int' overload that will throw if 'size' is out of range
  Args(int size, List args) : Args{checkInt(size), args} {}

  [[nodiscard]] auto size() const { return _size; }
  [[nodiscard]] const char* operator[](Size) const;
  [[nodiscard]] operator bool() const { return _size; }
private:
  [[nodiscard]] static Size checkInt(int);

  const Size _size{};
  List _args{};
};

} // namespace kanji_tools
