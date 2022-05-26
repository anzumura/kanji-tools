#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

namespace kanji_tools { //! \utils_group{Args}

//! \utils_class{Args} class for working with command line args
class Args {
public:
  using Size = uint16_t;           //!< type for arg count
  using List = const char* const*; //!< type for arg list

  Args() noexcept = default;  //!< default ctor, sets arg count to 0
  Args(const Args&) = delete; //!< deleted copy ctor

  //! ctor taking an `unsigned` arg count
  //! \param argc number of args
  //! \param argv list of args (pointer to pointer)
  //! \throw DomainError if `argc` is 0 and `argv` is non-null or vice versa
  Args(Size argc, List argv);

  //! ctor taking an `int` arg count, helpful in `main` functions
  //! \param argc number of args
  //! \param argv list of args (pointer to pointer)
  //! \throw RangeError if `argc` is negative or greater than max size
  Args(int argc, List argv) : Args{checkInt(argc), argv} {}

  //! ctor taking `const char*[]`, helpful in tests since it figures out size
  //! \tparam N size of the char array
  //! \param args list of args
  template<size_t N>
  constexpr Args(const char* (&args)[N]) noexcept // NOLINT: not explicit
      : _argc{N}, _argv{args} {
    static_assert(N <= std::numeric_limits<Size>::max());
  }

  //! return the arg at position `i`
  //! \throw RangeError if `i` is beyond the end of the arg list
  [[nodiscard]] const char* operator[](Size i) const;

  //! return total number of command line args
  [[nodiscard]] constexpr auto size() const noexcept { return _argc; }

  //! return true if there are command line args, i.e., #size is non-0
  [[nodiscard]] constexpr explicit operator bool() const noexcept {
    return _argc;
  }
private:
  [[nodiscard]] static Size checkInt(int);

  const Size _argc{};
  List _argv{};
};

//! \end_group
} // namespace kanji_tools
