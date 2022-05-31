#pragma once

#include <kanji_tools/utils/EnumContainer.h>

#include <array>

namespace kanji_tools { /// \utils_group{EnumMap}

/// base class for EnumMap that has a static #Empty value to share amongst
/// different EnumMaps \utils{EnumMap}
template<typename V> class BaseEnumMap {
public:
  inline static const V Empty{};
protected:
  BaseEnumMap() noexcept = default;
};

/// container class for mapping enum 'keys' to 'values' \utils{EnumMap}
///
/// The enum should have contiguous values starting at zero and a final 'None'
/// value which allows this class to use `std::array` internally instead of
/// `std::map`. It provides size(), operator[](), begin() and end() methods.
//
/// Passing T::None to const operator[]() returns an empty value, but T::None is
/// not valid for non-const operator[]() (it will throw an exception). Iteration
/// loops over only 'non-None' values. For example:
///
/// \code
///   // must have underlying type Enum::Size, start at 0 and final value 'None'
///   enum class Colors : Enum::Size { Red, Green, Blue, None };
///
///   // create and populate an EnumMap that maps 'Colors' to 'int'
///   EnumMap<Colors, int> m;
///   m[Colors::Red] = 2;
///   m[Colors::Green] = 4;
///   m[Colors::Blue] = 7;
///
///   for (auto i : m) std::cout << i << '\n'; // prints the 3 'int' values
///
///   // Colors::None can only used with a const instance of EnumMap
///   const auto& cm{m};
///   std::cout << cm[Colors::None]; // prints 0 (the default value for 'int')
/// \endcode
///
/// \tparam T 'key' type (scoped enum with contiguous values starting at zero)
/// \tparam V 'value' type (must have a default ctor)
/// \tparam N number of enum values (calculated via the position of T::None)
template<typename T, typename V, Enum::Size N = to_underlying(T::None),
    std::enable_if_t<is_scoped_enum_v<T>, int> = 0>
class EnumMap : public EnumContainer<T, N>, public BaseEnumMap<V> {
public:
  using base = EnumContainer<T, N>;

  EnumMap() noexcept = default;

  /// return 'value' for the given enum value `i` and allow it to be modified
  /// \throw RangeError if `i` is >= T::None
  [[nodiscard]] auto& operator[](T i) {
    return _values[base::checkIndex(i, base::IndexMsg)];
  }

  /// const operator[] const also supports returning an empty 'value' if `i` is
  /// the 'None' enum value (this is fine since it can't be modified)
  /// \throw RangeError if `i` > T::None
  [[nodiscard]] auto& operator[](T i) const {
    return i == T::None ? BaseEnumMap<V>::Empty
                        : _values[base::checkIndex(i, base::IndexMsg)];
  }

  /// return ConstIterator pointing at the start of the collection
  [[nodiscard]] auto begin() const noexcept { return ConstIterator{0, *this}; }

  /// return ConstIterator pointing at 'one-past' the end of the collection
  [[nodiscard]] auto end() const noexcept { return ConstIterator{N, *this}; }

  /// iterator for looping over 'values' in the collection
  class ConstIterator : public base::template Iterator<ConstIterator> {
  public:
    using iBase = typename base::template Iterator<ConstIterator>;
    using iBase::operator-, iBase::comparable, iBase::index, iBase::initialized;

    /// default ctor, iterator can't be used to get values since it hasn't been
    /// associated with a collection yet
    ConstIterator() noexcept : iBase{0} {}

    /// equal operator (forward iterator)
    /// \throw DomainError if iterators are from different EnumMap collections
    [[nodiscard]] bool operator==(const ConstIterator& x) const {
      comparable(_map == x._map);
      return iBase::operator==(x);
    }

    /// not-equal operator (forward iterator)
    /// \throw DomainError if iterators are from different EnumMap collections
    [[nodiscard]] bool operator!=(const ConstIterator& x) const {
      return !(*this == x);
    }

    /// return value at current location (input iterator)
    /// \throw DomainError if iterator hasn't been initialized
    /// \throw RangeError if location is invalid, i.e., at 'end' location
    [[nodiscard]] auto& operator*() const {
      initialized(_map);
      const auto i{index()};
      if (i >= N)
        iBase::rangeError(base::IndexMsg + std::to_string(i) + base::RangeMsg);
      return (*_map)[to_enum<T>(i)];
    }

    /// less-than operator (random-access iterator)
    /// \throw DomainError if iterators are from different EnumMap collections
    [[nodiscard]] bool operator<(const ConstIterator& x) const {
      comparable(_map == x._map);
      return index() < x.index();
    }

    /// less-than-or-equal operator (random-access iterator)
    /// \throw DomainError if iterators are from different EnumMap collections
    [[nodiscard]] bool operator<=(const ConstIterator& x) const {
      return *this < x || *this == x;
    }

    /// greater-than-or-equal operator (random-access iterator)
    /// \throw DomainError if iterators are from different EnumMap collections
    [[nodiscard]] bool operator>=(const ConstIterator& x) const {
      return !(*this < x);
    }

    /// greater-than operator (random-access iterator)
    /// \throw DomainError if iterators are from different EnumMap collections
    [[nodiscard]] bool operator>(const ConstIterator& x) const {
      return !(*this <= x);
    }

    /// return difference between iterators (random-access iterator)
    /// \throw DomainError if iterators are from different EnumMap collections
    [[nodiscard]] auto operator-(const ConstIterator& x) const {
      comparable(_map == x._map);
      return index() - x.index();
    }
  private:
    friend EnumMap<T, V>; // calls private ctor

    ConstIterator(Enum::Size i, const EnumMap<T, V>& m) noexcept
        : iBase{i}, _map{&m} {}

    const EnumMap<T, V>* _map{};
  };
private:
  std::array<V, N> _values{};
};

/// \end_group
} // namespace kanji_tools
