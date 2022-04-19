#pragma once

#include <kanji_tools/utils/EnumTraits.h>

#include <compare>
#include <concepts>
#include <string>

namespace kanji_tools {

class BaseIterableEnum {
public:
  using Index = size_t;

  BaseIterableEnum(const BaseIterableEnum&) = delete;
  BaseIterableEnum& operator=(const BaseIterableEnum&) = delete;
protected:
  inline static const std::string IndexMsg{"index '"}, EnumMsg{"enum '"},
      RangeMsg{"' is out of range"};

  class BaseIterator {
  public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;

    // operator<=> enables == and != needed for 'input interator' and <, >, <=
    // and >= needed for 'random access iterator'
    [[nodiscard]] auto operator<=>(
        const BaseIterator&) const noexcept = default;
  protected:
    inline static const std::string BadBegin{"can't decrement past zero"},
        BadEnd{"can't increment past end"};

    // helper functions for throwing errors if the value passed in is 'false'
    static void comparable(bool);
    static void initialized(bool);

    static void rangeError(const std::string&);

    explicit BaseIterator(Index index) noexcept;

    [[nodiscard]] Index& index();
    [[nodiscard]] Index index() const;
  private:
    Index _index;
  };

  static void rangeError(const std::string&);

  BaseIterableEnum() noexcept = default;
};

// 'to_index' and 'from_index' convert between 'size_t' and a scoped enum value.
// These functions are used in EnumMap and EnumArray classes (where the scoped
// enum is used as an index) and are safer than plain static cast since there
// more compile time type checks on the types.

template<typename T>
[[nodiscard]] constexpr std::enable_if_t<is_scoped_enum_v<T>, size_t> to_index(
    T t) noexcept {
  static_assert(sizeof(T) <= sizeof(size_t));
  return static_cast<size_t>(t);
}

template<typename T>
[[nodiscard]] constexpr std::enable_if_t<is_scoped_enum_v<T>, T> from_index(
    size_t i) noexcept {
  static_assert(sizeof(T) <= sizeof(size_t));
  return static_cast<T>(i);
}

// IterableEnum is a base class for EnumArray and EnumMap classes. It provides
// 'size' and 'getIndex' methods and an 'Iterator' (used by derived classes)
template<typename T, size_t N> class IterableEnum : public BaseIterableEnum {
public:
  [[nodiscard]] static constexpr size_t size() noexcept { return N; }
protected:
  [[nodiscard]] static auto getIndex(T x) {
    return checkIndex(to_underlying(x), EnumMsg);
  }

  IterableEnum() noexcept = default;

  // Random access iterator for looping over all values of T (the scoped enum).
  template<typename Derived> class Iterator : public BaseIterator {
  public:
    // common requirements for iterators

    auto& operator++() {
      if (index() >= N) rangeError(BadEnd);
      ++index();
      return derived();
    }

    auto operator++(int) {
      Derived x{derived()};
      ++*this;
      return x;
    }

    // bi-directional iterator requirements

    auto& operator--() {
      if (!index()) rangeError(BadBegin);
      --index();
      return derived();
    }

    auto operator--(int) {
      Derived x{derived()};
      --*this;
      return x;
    }

    // random-access iterator requirements (except non-const operator[])

    [[nodiscard]] auto operator[](difference_type i) const {
      return *(derived() + i);
    }

    auto& operator+=(difference_type i) {
      if ((i += static_cast<difference_type>(index())) < 0)
        rangeError(BadBegin);
      if (const auto j{static_cast<Index>(i)}; j > N)
        rangeError(BadEnd);
      else
        index() = j;
      return derived();
    }

    auto& operator-=(difference_type i) { return *this += -i; }

    [[nodiscard]] auto operator+(difference_type i) const {
      Derived x{derived()};
      return x += i;
    }

    [[nodiscard]] auto operator-(difference_type i) const {
      Derived x{derived()};
      return x -= i;
    }
  protected:
    explicit Iterator(Index index) noexcept : BaseIterator{index} {}
  private:
    [[nodiscard]] auto& derived() noexcept {
      return static_cast<Derived&>(*this);
    }

    [[nodiscard]] auto& derived() const noexcept {
      return static_cast<const Derived&>(*this);
    }
  };

  template<typename I>
  requires std::integral<I> || std::same_as<T, I>
  [[nodiscard]] static auto checkIndex(I i, const std::string& name) {
    const auto x{static_cast<Index>(i)};
    if (x >= N) {
      std::string msg{name};
      if constexpr (std::is_same_v<I, T>)
        msg += "enum value " + std::to_string(x);
      else // use original value in error message (so int '-1' is preserved)
        msg += std::to_string(i);
      rangeError(msg + RangeMsg);
    }
    return x;
  }
};

} // namespace kanji_tools
