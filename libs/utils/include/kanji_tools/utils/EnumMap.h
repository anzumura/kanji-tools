#pragma once

#include <kanji_tools/utils/IterableEnum.h>

#include <array>

namespace kanji_tools {

// EnumMap is a collection class for mapping scoped enum 'keys' to 'values'. The
// enum should have contiguous values starting at zero and a final 'None' which
// allows this class to use 'std::array' internally instead of 'std::map'. It
// provides 'size', 'operator[]', 'begin' and 'end' methods.
//
// Passing T::None to const 'operator[]' returns an empty value, but T::None is
// not valid for non-const 'operator[]' (it will throw an exception). Iteration
// loops over only 'non-None' values. For example:
//
// enum class Colors { Red, Green, Blue, None };
// EnumMap<Colors, int> m;
// m[Colors::Red] = 2;
// m[Colors::Green] = 4;
// m[Colors::Blue] = 7;
// for (auto i : m) std::cout << i << '\n'; // prints the 3 values
// const auto& cm{m};
// std::cout << cm[Colors::None]; // prints 0 (the default value for 'int')

// BaseEnumMap has a static 'Empty' value to share amongst different EnumMaps
template<typename V> class BaseEnumMap {
public:
  inline static const V Empty{};
protected:
  BaseEnumMap() noexcept = default;
};

template<typename T, typename V, std::enable_if_t<is_scoped_enum_v<T>, int> = 0>
class EnumMap : public IterableEnum<T, static_cast<size_t>(T::None)>,
                public BaseEnumMap<V> {
private:
  static constexpr auto N{static_cast<size_t>(T::None)};
  using base = IterableEnum<T, N>;

  std::array<V, N> _values;
public:
  EnumMap() noexcept = default;

  // return 'value' for the given enum value 'i' and allow it to be modified
  [[nodiscard]] auto& operator[](T i) {
    return _values[base::checkIndex(i, base::Index)];
  }

  // operator[] const version also supports returning an empty 'value' if 'i' is
  // the 'None' enum value (this is fine since it can't be modified).
  [[nodiscard]] auto& operator[](T i) const {
    return i == T::None ? BaseEnumMap<V>::Empty
                        : _values[base::checkIndex(i, base::Index)];
  }

  class ConstIterator : public base::template Iterator<ConstIterator> {
  private:
    friend EnumMap<T, V>;
    using iBase = typename base::template Iterator<ConstIterator>;

    ConstIterator(size_t index, const EnumMap<T, V>& map) noexcept
        : iBase{index}, _map{&map} {}

    void checkComparable(const ConstIterator& x) const {
      if (_map != x._map) throw std::domain_error{"not comparable"};
    }

    const EnumMap<T, V>* _map{};
  public:
    // forward iterator requirements (a default constructor)
    ConstIterator() noexcept : iBase{0} {}

    [[nodiscard]] bool operator==(const ConstIterator& x) const {
      checkComparable(x);
      return iBase::operator==(x);
    }
    [[nodiscard]] bool operator<(const ConstIterator& x) const {
      checkComparable(x);
      return iBase::index() < x.index();
    }
    [[nodiscard]] bool operator!=(const ConstIterator& x) const {
      return !(*this == x);
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

    // input iterator requirements (except operator->)
    [[nodiscard]] auto& operator*() const {
      if (!_map) throw std::domain_error{"not initialized"};
      if (iBase::index() >= N)
        iBase::error(
            base::Index + std::to_string(iBase::index()) + base::Range);
      return (*_map)[static_cast<T>(iBase::index())];
    }

    // random-access iterator requirements
    [[nodiscard]] auto operator-(const ConstIterator& x) const {
      checkComparable(x);
      return iBase::index() - x.index();
    }
  };

  // only support 'const' iteration
  [[nodiscard]] auto begin() const noexcept { return ConstIterator{0, *this}; }
  [[nodiscard]] auto end() const noexcept { return ConstIterator{N, *this}; }
};

} // namespace kanji_tools
