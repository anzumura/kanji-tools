#pragma once

#include <kt_utils/String.h>
#include <kt_utils/TypeTraits.h>

#include <compare>
#include <concepts>

namespace kanji_tools { /// \utils_group{EnumContainer}
/// base classes used by EnumMap and EnumList

/// non-templated base class for EnumContainer \utils{EnumContainer}
///
/// This class prevents copying and has a #Size type that needs to be used by
/// scoped enums participating in the containers. A small unsigned type is used
/// to make sure enums don't have negative values (they are supposed to have
/// contiguous values starting at zero).
class Enum {
public:
  using Size = uint8_t;

  Enum(const Enum&) = delete;
  auto operator=(const Enum&) = delete;
protected:
  inline static const String IndexMsg{"index '"}, EnumMsg{"enum '"},
      RangeMsg{"' is out of range"};

  /// non-templated base class for EnumContainer::Iterator \utils{EnumContainer}
  class BaseIterator {
  public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;

    /// operator<=> enables == and != needed for 'input interator' and <, >, <=
    /// and >= needed for 'random access iterator'
    [[nodiscard]] auto operator<=>(
        const BaseIterator&) const noexcept = default; // NOLINT: nullptr
  protected:
    inline static const String BadBegin{"can't decrement past zero"},
        BadEnd{"can't increment past end"};

    /// helper functions for throwing errors if value passed in is 'false' @{
    static void comparable(bool);
    static void initialized(bool); ///@}

    static void rangeError(const String&);

    explicit BaseIterator(Size index) noexcept;

    [[nodiscard]] Size& index();
    [[nodiscard]] Size index() const;
  private:
    Size _index;
  };

  static void rangeError(const String&);

  Enum() noexcept = default;
};

/// templated base class for EnumList and EnumMap \utils{EnumContainer}
///
/// This class provides size() and getIndex() methods and an Iterator with
/// functionality that is further enhanced by the derived class iterators.
template<scoped_enum T, Enum::Size N> class EnumContainer : public Enum {
public:
  [[nodiscard]] static constexpr auto size() noexcept { return N; }
protected:
  [[nodiscard]] static auto getIndex(T x) {
    return checkIndex(to_underlying(x), EnumMsg);
  }

  /// ctor makes sure enum T has the same underlying type as Size since derived
  /// classes (like EnumList) are designed to only support up to Size items
  EnumContainer() noexcept {
    static_assert(std::is_same_v<std::underlying_type_t<T>, Size>);
  }

  /// Random access iterator for looping over values of T \utils{EnumContainer}
  template<typename Derived> class Iterator : public BaseIterator {
  public:
    /// prefix increment operator (common for all iterators)
    /// \throw RangeError if iterator is at the 'end' location
    auto& operator++();

    /// postfix increment operator (common for all iterators)
    /// \throw RangeError if iterator is at 'end' location
    auto operator++(int);

    /// prefix decrement operator (bi-directional iterator)
    /// \throw RangeError if iterator is at the beginning
    auto& operator--();

    /// postfix decrement operator (bi-directional iterator)
    /// \throw RangeError if iterator is at the beginning
    auto operator--(int);

    /// return value at location `i` (random-access iterator)
    /// \throw RangeError if `i` is out of range
    [[nodiscard]] auto operator[](difference_type i) const;

    /// move iterator forward by `i` (random-access iterator)
    /// \throw RangeError if new location would be beyond 'end' location
    auto& operator+=(difference_type i);

    /// move iterator backward by `i` (random-access iterator)
    /// \throw RangeError if new location would be before the beginning location
    auto& operator-=(difference_type i);

    /// return a new iterator at current location + `i` (random-access iterator)
    /// \throw RangeError if new location would be beyond 'end' location
    [[nodiscard]] auto operator+(difference_type i) const;

    /// return a new iterator at current location - `i` (random-access iterator)
    /// \throw RangeError if new location would be before the beginning location
    [[nodiscard]] auto operator-(difference_type i) const;
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
  [[nodiscard]] static auto checkIndex(I i, const String& name);
};

// Out-of-class template definitions

template<scoped_enum T, Enum::Size N> template<typename I>
requires std::integral<I> || std::same_as<T, I>
auto EnumContainer<T, N>::checkIndex(I i, const String& name) {
  const auto x{static_cast<Size>(i)};
  if (x >= N) {
    String msg{name};
    if constexpr (std::is_same_v<I, T>)
      msg += "enum value " + std::to_string(x);
    else // use original value in error message (so int '-1' is preserved)
      msg += std::to_string(i);
    rangeError(msg + RangeMsg);
  }
  return x;
}

template<scoped_enum T, Enum::Size N>
template<typename Derived>
auto& EnumContainer<T, N>::Iterator<Derived>::operator++() {
  if (index() >= N) rangeError(BadEnd);
  ++index();
  return derived();
}

template<scoped_enum T, Enum::Size N>
template<typename Derived>
auto EnumContainer<T, N>::Iterator<Derived>::operator++(int) {
  const auto x{derived()};
  ++*this;
  return x;
}

template<scoped_enum T, Enum::Size N>
template<typename Derived>
auto& EnumContainer<T, N>::Iterator<Derived>::operator--() {
  if (!index()) rangeError(BadBegin);
  --index();
  return derived();
}

template<scoped_enum T, Enum::Size N>
template<typename Derived>
auto EnumContainer<T, N>::Iterator<Derived>::operator--(int) {
  const auto x{derived()};
  --*this;
  return x;
}

template<scoped_enum T, Enum::Size N>
template<typename Derived>
auto EnumContainer<T, N>::Iterator<Derived>::operator[](
    difference_type i) const {
  return *(derived() + i);
}

template<scoped_enum T, Enum::Size N>
template<typename Derived>
auto& EnumContainer<T, N>::Iterator<Derived>::operator+=(difference_type i) {
  if (difference_type curIndex{index()}; (curIndex += i) < 0)
    rangeError(BadBegin);
  else if (const auto j{static_cast<Size>(curIndex)}; j > N)
    rangeError(BadEnd);
  else
    index() = j;
  return derived();
}

template<scoped_enum T, Enum::Size N>
template<typename Derived>
auto& EnumContainer<T, N>::Iterator<Derived>::operator-=(difference_type i) {
  return *this += -i;
}

template<scoped_enum T, Enum::Size N>
template<typename Derived>
auto EnumContainer<T, N>::Iterator<Derived>::operator+(
    difference_type i) const {
  auto x{derived()};
  return x += i;
}

template<scoped_enum T, Enum::Size N>
template<typename Derived>
auto EnumContainer<T, N>::Iterator<Derived>::operator-(
    difference_type i) const {
  auto x{derived()};
  return x -= i;
}

/// \end_group
} // namespace kanji_tools
