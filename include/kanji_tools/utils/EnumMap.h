#pragma once

#include <kanji_tools/utils/IterableEnum.h>

#include <array>

namespace kanji_tools {

// EnumMap is a collection class for mapping scoped enum 'keys' to 'values'. The
// enum should have contiguous values starting at zero and a final 'None' which
// allows this class to use 'std::array' internally instead of 'std::map'. It
// provides 'size', 'operator[]', 'begin' and 'end' methods.
//
// Passing T::None to 'operator[] const' returns an empty value, but T::None is
// not valid for non-const 'operator[]' (it will throw an exception). Iteration
// loops over 'non-None' values. For example:
//
// enum class Colors { Red, Green, Blue, None };
// EnumMap<Colors, int> m;
// m[Colors::Red] = 2;
// m[Colors::Green] = 4;
// m[Colors::Blue] = 7;
// for (auto i : m) std::cout << i << '\n'; // prints the 3 values
// std::cout << m[Colors::None]; // prints 0 (the default value for 'int')

template<typename V> class BaseEnumMap {
protected:
  BaseEnumMap() noexcept = default;

  inline static const V Empty{};
};

template<typename T, typename V, std::enable_if_t<is_scoped_enum_v<T>, int> = 0>
class EnumMap : public IterableEnum<T, static_cast<size_t>(T::None)>,
                public BaseEnumMap<V> {
private:
  static constexpr auto N = static_cast<size_t>(T::None);
  using base = IterableEnum<T, N>;

  std::array<V, N> _values;
public:
  class Iterator : public base::template BaseIterator<Iterator> {
  private:
    using iBase = typename base::template BaseIterator<Iterator>;

    const EnumMap<T, V>* _map;
  public:
    // forward iterator requirements (a default constructor)
    Iterator(size_t index = 0, const EnumMap<T, V>* map = nullptr) noexcept
        : iBase(index), _map(map) {}

    // input iterator requirements (except operator->)
    [[nodiscard]] auto& operator*() const {
      if (!_map) throw std::domain_error("not initialized");
      if (iBase::_index >= N)
        iBase::error(base::Index + std::to_string(iBase::_index) + base::Range);
      return (*_map)[static_cast<T>(iBase::_index)];
    }

    // random-access iterator requirements
    [[nodiscard]] auto operator[](typename iBase::difference_type i) const {
      return *(*this + i);
    }
  };

  // only support 'const' iteration
  [[nodiscard]] auto begin() const noexcept { return Iterator(0, this); }
  [[nodiscard]] auto end() const noexcept { return Iterator(N, this); }

  [[nodiscard]] auto& operator[](T i) const {
    return i == T::None ? BaseEnumMap<V>::Empty
                        : _values[base::checkIndex(i, base::Index)];
  }
  [[nodiscard]] auto& operator[](T i) {
    return _values[base::checkIndex(i, base::Index)];
  }
};

} // namespace kanji_tools
