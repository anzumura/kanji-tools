#pragma once

#include <kanji_tools/utils/EnumTraits.h>

#include <compare>
#include <stdexcept>
#include <string>

namespace kanji_tools {

// IterableEnum is a base class for EnumArray and EnumMap classes. It provides
// 'size' and 'getIndex' methods and a 'BaseIterator' (used by derived classes)
template<typename T, size_t N> class IterableEnum {
public:
  IterableEnum(const IterableEnum&) = delete;
  IterableEnum& operator=(const IterableEnum&) = delete;

  [[nodiscard]] static constexpr size_t size() noexcept { return N; }
protected:
  inline static const std::string Index{"index '"}, Enum{"enum '"},
    Range{"' is out of range"};

  [[nodiscard]] static auto getIndex(T x) {
    return checkIndex(to_underlying(x), Enum);
  }

  IterableEnum() noexcept = default;

  // Random access iterator for looping over all values of T (the scoped enum).
  template<typename Derived> class BaseIterator {
  public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;

    // common requirements for iterators
    auto& operator++() {
      if (_index >= N) error(BadEnd);
      ++_index;
      return derived();
    }
    auto operator++(int) {
      Derived x = derived();
      ++*this;
      return x;
    }

    // operator<=> enables == and != needed for 'input interator' and <, >, <=
    // and >= needed for 'random access iterator'
    [[nodiscard]] auto
    operator<=>(const BaseIterator& x) const noexcept = default;

    // bi-directional iterator requirements
    auto& operator--() {
      if (_index == 0) error(BadBegin);
      --_index;
      return derived();
    }
    auto operator--(int) {
      Derived x = derived();
      --*this;
      return x;
    }

    // random-access iterator requirements (except non-const operator[])
    auto& operator+=(difference_type i) {
      if ((i += static_cast<difference_type>(_index)) < 0) error(BadBegin);
      if (const auto j = static_cast<size_t>(i); j > N)
        error(BadEnd);
      else
        _index = j;
      return derived();
    }
    auto& operator-=(difference_type i) { return *this += -i; }
    [[nodiscard]] auto operator+(difference_type i) const {
      Derived x = derived();
      return x += i;
    }
    [[nodiscard]] auto operator-(difference_type i) const {
      Derived x = derived();
      return x -= i;
    }
    [[nodiscard]] auto operator-(const BaseIterator& x) const noexcept {
      return _index - x._index;
    }
  protected:
    static void error(const std::string& s) { throw std::out_of_range(s); }

    BaseIterator(size_t index) noexcept : _index(index) {}

    size_t _index{};
  private:
    [[nodiscard]] auto& derived() noexcept {
      return static_cast<Derived&>(*this);
    }
    [[nodiscard]] auto& derived() const noexcept {
      return static_cast<const Derived&>(*this);
    }

    inline static const std::string BadBegin{"can't decrement past zero"},
      BadEnd{"can't increment past end"};
  };

  template<typename Index>
  [[nodiscard]] static auto checkIndex(Index i, const std::string& name) {
    const auto x = static_cast<size_t>(i);
    if (x >= N) {
      std::string msg = name;
      if constexpr (std::is_same_v<Index, T>)
        msg += "enum value " + std::to_string(x);
      else // use original value in error message (so int '-1' is preserved)
        msg += std::to_string(i);
      throw std::out_of_range(msg + Range);
    }
    return x;
  }
};

} // namespace kanji_tools
