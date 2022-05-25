#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

namespace kanji_tools {

//! \lib_utils{Args} class for working with command line args
class Args {
public:
  using Size = uint16_t;           //!< command line arg count
  using List = const char* const*; //!< command line arg list

  Args() noexcept = default;  //!< default ctor, sets arg count to 0
  Args(const Args&) = delete; //!< deleted copy ctor

  //! ctor taking an `unsigned` arg count
  //! \param argc number of args
  //! \param argv list of args (pointer to pointer)
  //! \throw std::domain_error if \p argc is 0 and \p argv is non-null or if
  //!                          \p argc is non-0 and \p argv is null
  Args(Size argc, List argv);

  //! ctor taking an `int` arg count, helpful in `main` functions
  //! \param argc number of args
  //! \param argv list of args (pointer to pointer)
  //! \throw std::range_error if \p argc is negative or greater than max size
  Args(int argc, List argv) : Args{checkInt(argc), argv} {}

  //! ctor taking `const char*[]`, helpful in tests since it figures out size
  //! \tparam N size of the char array
  //! \param args list of args
  template<size_t N>
  constexpr Args(const char* (&args)[N]) noexcept // NOLINT: not explicit
      : _argc{N}, _argv{args} {
    static_assert(N <= std::numeric_limits<Size>::max());
  }

  //! return the arg at position \p i
  //! \throw std::range_error if \p i is beyond the end of the arg list
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

} // namespace kanji_tools
