#pragma once

#include <kanji_tools/utils/EnumContainer.h>

#include <array>

namespace kanji_tools {

// EnumMap is a container class for mapping scoped enum 'keys' to 'values'. The
// enum should have contiguous values starting at zero and a final 'None' which
// allows this class to use 'std::array' internally instead of 'std::map'. It
// provides 'size', 'operator[]', 'begin' and 'end' methods.
//
// Passing T::None to const 'operator[]' returns an empty value, but T::None is
// not valid for non-const 'operator[]' (it will throw an exception). Iteration
// loops over only 'non-None' values. For example:
//
//   enum class Colors : Enum::Size { Red, Green, Blue, None };
//   EnumMap<Colors, int> m;
//   m[Colors::Red] = 2;
//   m[Colors::Green] = 4;
//   m[Colors::Blue] = 7;
//   for (auto i : m) std::cout << i << '\n'; // prints the 3 'int' values
//   const auto& cm{m};
//   std::cout << cm[Colors::None]; // prints 0 (the default value for 'int')

// BaseEnumMap has a static 'Empty' value to share amongst different EnumMaps
template<typename V> class BaseEnumMap {
public:
  inline static const V Empty{};
protected:
  BaseEnumMap() noexcept = default;
};

template<typename T, typename V, Enum::Size N = to_underlying(T::None),
    std::enable_if_t<is_scoped_enum_v<T>, int> = 0>
class EnumMap : public EnumContainer<T, N>, public BaseEnumMap<V> {
public:
  using base = EnumContainer<T, N>;

  EnumMap() noexcept = default;

  // return 'value' for the given enum value 'i' and allow it to be modified
  [[nodiscard]] auto& operator[](T i) {
    return _values[base::checkIndex(i, base::IndexMsg)];
  }

  // operator[] const version also supports returning an empty 'value' if 'i' is
  // the 'None' enum value (this is fine since it can't be modified).
  [[nodiscard]] auto& operator[](T i) const {
    return i == T::None ? BaseEnumMap<V>::Empty
                        : _values[base::checkIndex(i, base::IndexMsg)];
  }

  // only support 'const' iteration
  [[nodiscard]] auto begin() const noexcept { return ConstIterator{0, *this}; }
  [[nodiscard]] auto end() const noexcept { return ConstIterator{N, *this}; }

  class ConstIterator : public base::template Iterator<ConstIterator> {
  public:
    // base iterator implements some operations such as prefix and postfix
    // increment and decrement, operator[], +=, -=, + and -.
    using iBase = typename base::template Iterator<ConstIterator>;

    // forward iterator requirements (default ctor, equal)

    ConstIterator() noexcept : iBase{0} {}

    [[nodiscard]] bool operator==(const ConstIterator& x) const {
      iBase::comparable(_map == x._map);
      return iBase::operator==(x);
    }

    [[nodiscard]] bool operator!=(const ConstIterator& x) const {
      return !(*this == x);
    }

    // input iterator requirements (except operator->)

    [[nodiscard]] auto& operator*() const {
      iBase::initialized(_map);
      const auto i{iBase::index()};
      if (i >= N)
        iBase::rangeError(base::IndexMsg + std::to_string(i) + base::RangeMsg);
      return (*_map)[to_enum<T>(i)];
    }

    // random-access iterator requirements (compare, subtracting iterators)

    [[nodiscard]] bool operator<(const ConstIterator& x) const {
      iBase::comparable(_map == x._map);
      return iBase::index() < x.index();
    }

    [[nodiscard]] bool operator<=(const ConstIterator& x) const {
      return *this < x || *this == x;
    }

    [[nodiscard]] bool operator>=(const ConstIterator& x) const {
      return !(*this < x);
    }

    [[nodiscard]] bool operator>(const ConstIterator& x) const {
      return !(*this <= x);
    }

    using iBase::operator-;
    [[nodiscard]] auto operator-(const ConstIterator& x) const {
      iBase::comparable(_map == x._map);
      return iBase::index() - x.index();
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

} // namespace kanji_tools
