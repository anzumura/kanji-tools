#pragma once

#include <kanji_tools/utils/EnumTraits.h>

#include <compare>
#include <concepts>
#include <string>

namespace kanji_tools {

class BaseEnum {
public:
  // enforce using a small unsigned type to help make sure scoped enums being
  // used in derived classes (EnumMap, EnumArray, etc.) don't have negative
  // values (they are supposed to be contiguous values starting at zero).
  using Size = uint8_t;

  BaseEnum(const BaseEnum&) = delete;
  BaseEnum& operator=(const BaseEnum&) = delete;
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
        const BaseIterator&) const noexcept = default; // NOLINT: nullptr
  protected:
    inline static const std::string BadBegin{"can't decrement past zero"},
        BadEnd{"can't increment past end"};

    // helper functions for throwing errors if the value passed in is 'false'
    static void comparable(bool);
    static void initialized(bool);

    static void rangeError(const std::string&);

    explicit BaseIterator(Size index) noexcept;

    [[nodiscard]] Size& index();
    [[nodiscard]] Size index() const;
  private:
    Size _index;
  };

  static void rangeError(const std::string&);

  BaseEnum() noexcept = default;
};

// IterableEnum is a base class for EnumArray and EnumMap classes. It provides
// 'size' and 'getIndex' methods and an 'Iterator' (used by derived classes)
template<typename T, BaseEnum::Size N> class IterableEnum : public BaseEnum {
public:
  [[nodiscard]] static constexpr Size size() noexcept { return N; }
protected:
  [[nodiscard]] static auto getIndex(T x) {
    return checkIndex(to_underlying(x), EnumMsg);
  }

  IterableEnum() noexcept {
    // make sure enum 'T' has the same underlying type as 'Size' since derived
    // classes (like EnumArray) are designed to only support up to 'Size' items
    static_assert(std::is_same_v<std::underlying_type_t<T>, Size>);
  }

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
      const auto x{derived()};
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
      const auto x{derived()};
      --*this;
      return x;
    }

    // random-access iterator requirements (except non-const operator[])

    [[nodiscard]] auto operator[](difference_type i) const {
      return *(derived() + i);
    }

    auto& operator+=(difference_type i) {
      if (difference_type curIndex{index()}; (curIndex += i) < 0)
        rangeError(BadBegin);
      else if (const auto j{static_cast<Size>(curIndex)}; j > N)
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
    explicit Iterator(Size index) noexcept : BaseIterator{index} {}
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
    const auto x{static_cast<Size>(i)};
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
