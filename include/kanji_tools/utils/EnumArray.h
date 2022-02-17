#ifndef KANJI_TOOLS_UTILS_ENUM_ARRAY_H
#define KANJI_TOOLS_UTILS_ENUM_ARRAY_H

#include <kanji_tools/utils/EnumTraits.h>

#include <array>
#include <iostream>
#include <map>
#include <string>

namespace kanji_tools {

// EnumArray is a helper class for scoped enums that have contiguous values starting at zero
// and a final value of 'None'. It provides 'size', 'operator[]' and 'fromString' methods as
// well as 'begin' and 'end' methods for range based iteration over the enum values. There is
// also a 'toString' method that is used by aot-of-class 'toString' and ostream 'operator<<'
// functions. There are also 'hasValue' and 'operator!' global functions based on T::None. In
// order to enable this functionality 'is_enumarray' must be set to 'true' and an instance of
// EnumArray must be created with string values in the same order as the scoped enum values.
//
// Here's an example of how to create an EnumArray:
//
// enum class Colors { Red, Green, Blue, None };
// template<> inline constexpr bool is_enumarray<Colors> = true;
// inline const auto AllColors = BaseEnumArray<Colors>::create("Red", "Green", "Blue");
//
// for (auto c : AllColors) { std::cout << c << '\n'; } // prints each color including "None"
//
// Note, "None" should not be passed to 'initialize'. Instead there is a static_assert that
// ensures T::None is the next value after the list of strings (this helps ensure the enum
// and the list of names stay in sync).

// 'is_enumarray' bool that should be specialized:
template<typename T, std::enable_if_t<is_scoped_enum_v<T>, int> = 0> inline constexpr bool is_enumarray = false;

template<typename T> class BaseEnumArray {
public:
  // must specifiy at least one 'name' when calling 'create' (see comments above)
  template<typename... Names> [[nodiscard]] static auto create(const char* name, Names...);

  [[nodiscard]] static const auto& instance() {
    if (!_instance) throw std::domain_error("must call 'create' before calling 'instance'");
    return *_instance;
  }

  BaseEnumArray(const BaseEnumArray&) = delete;
  BaseEnumArray& operator=(const BaseEnumArray&) = delete;

  [[nodiscard]] virtual const std::string& toString(T) const = 0;
protected:
  BaseEnumArray() {}

  inline static BaseEnumArray<T>* _instance = nullptr;
};

template<typename T, size_t N> class EnumArray : public BaseEnumArray<T> {
public:
  // Random access iterator for looping over all values of T (the scoped enum) including the final
  // 'None' entry. This iterator does not allow modifying entries.
  class Iterator {
  public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = const T;
    using pointer = const T*;
    using reference = const T&;
    using self_type = Iterator;

    // forward iterator requirements (a default constructor)
    Iterator(size_t index = 0) noexcept : _index(index) {}
    // common requirements for iterators
    auto& operator++() {
      // entry at 'A' is 'T::None' so allow iterating until index == N to include it
      if (_index > N) throw std::out_of_range("can't increment past end");
      ++_index;
      return *this;
    }
    auto operator++(int) {
      Iterator x = *this;
      ++*this;
      return x;
    }
    // input iterator requirements (except operator->)
    [[nodiscard]] auto operator==(const Iterator& x) const noexcept { return _index == x._index; }
    [[nodiscard]] auto operator!=(const Iterator& x) const noexcept { return !(*this == x); }
    [[nodiscard]] auto operator*() const {
      // exception should only happen when dereferencing 'end' since other methods prevent moving out of range
      if (_index > N) throw std::out_of_range("index '" + std::to_string(_index) + "' is out of range");
      return static_cast<T>(_index);
    }
    // bi-directional iterator requirements
    auto& operator--() {
      if (_index == 0) throw std::out_of_range("can't decrement past zero");
      --_index;
      return *this;
    }
    auto operator--(int) {
      Iterator x = *this;
      --*this;
      return x;
    }
    // random-access iterator requirements (except non-const operator[])
    auto& operator+=(difference_type offset) {
      if (_index + offset < 0 || _index + offset > N + 1) throw std::out_of_range("can't increment past end");
      _index += offset;
      return *this;
    }
    auto& operator-=(difference_type offset) { return *this += -offset; }
    [[nodiscard]] auto operator[](difference_type offset) const { return *(*this + offset); }
    [[nodiscard]] auto operator+(difference_type offset) const {
      Iterator x = *this;
      return x += offset;
    }
    [[nodiscard]] auto operator-(difference_type offset) const {
      Iterator x = *this;
      return x -= offset;
    }
    [[nodiscard]] auto operator-(const Iterator& x) const noexcept { return _index - x._index; }
    [[nodiscard]] auto operator<(const Iterator& x) const noexcept { return _index < x._index; }
    [[nodiscard]] auto operator>(const Iterator& x) const noexcept { return x < *this; }
    [[nodiscard]] auto operator<=(const Iterator& x) const noexcept { return !(x < *this); }
    [[nodiscard]] auto operator>=(const Iterator& x) const noexcept { return !(*this < x); }
  private:
    size_t _index = 0;
  };

  [[nodiscard]] static auto begin() noexcept { return Iterator(0); }
  [[nodiscard]] static auto end() noexcept { return Iterator(N + 1); } // 'N' is 'T::None' so end should be N + 1

  [[nodiscard]] static constexpr size_t size() noexcept { return N + 1; }

  [[nodiscard]] auto operator[](size_t i) const {
    if (i > N) throw std::out_of_range("index '" + std::to_string(i) + "' is out of range");
    return static_cast<T>(i);
  }

  [[nodiscard]] auto fromString(const std::string& s, bool allowEmptyAsNone = false) const {
    if (allowEmptyAsNone && s.empty() || s == None) return T::None;
    const auto i = _nameMap.find(s);
    if (i == _nameMap.end()) throw std::domain_error("name '" + s + "' not found");
    return i->second;
  }

  [[nodiscard]] const std::string& toString(T x) const override {
    size_t i = to_underlying(x);
    if (i > N) throw std::out_of_range("enum '" + std::to_string(i) + "' is out of range");
    return i < N ? _names[i] : None;
  }
private:
  inline const static std::string None = "None";
  friend BaseEnumArray<T>; // static 'EnumArray<T>::initialize' method calls private constructor

  void insert(const char* name, size_t index) {
    const auto i = _nameMap.emplace(name, static_cast<T>(index));
    if (!i.second) throw std::domain_error("duplicate name '" + i.first->first + "'");
    if (i.first->first == None) throw std::domain_error("'None' should not be specified");
    _names[index] = i.first->first;
  }

  EnumArray(const char* name) { insert(name, N - 1); }

  template<typename... Names> EnumArray(const char* name, Names... args) : EnumArray(args...) {
    insert(name, N - 1 - sizeof...(args));
    // set _instance after processing all 'args' in case an exception is thrown
    if (sizeof...(args) == N - 1) BaseEnumArray<T>::_instance = this;
  }

  std::array<std::string, N> _names;
  std::map<std::string, T> _nameMap;
};

template<typename T>
template<typename... Names>
[[nodiscard]] auto BaseEnumArray<T>::create(const char* name, Names... args) {
  static_assert(is_enumarray<T>, "need to define 'is_enumarray' for T before calling 'initialize'");
  // Make sure the scoped enum 'T' has a value of None that is just past the set of string
  // values provided - this will help ensure that any changes to the enum must also be made
  // to the call to 'create' and vice versa.
  static_assert(static_cast<T>(sizeof...(Names) + 1) == T::None);
  if (_instance) throw std::domain_error("'create' should only be called once");
  return EnumArray<T, sizeof...(args) + 1>(name, args...);
}

template<typename T> [[nodiscard]] std::enable_if_t<is_enumarray<T>, const std::string&> toString(T x) {
  return BaseEnumArray<T>::instance().toString(x);
}

template<typename T> std::enable_if_t<is_enumarray<T>, std::ostream&> operator<<(std::ostream& os, T x) {
  return os << toString(x);
}

template<typename T> [[nodiscard]] constexpr std::enable_if_t<is_enumarray<T>, bool> hasValue(T x) noexcept {
  return x != T::None;
}

template<typename T> [[nodiscard]] constexpr std::enable_if_t<is_enumarray<T>, bool> operator!(T x) noexcept {
  return !hasValue(x);
}

// 'isNextNone' returns true if the next value after 'x' is T::None
template<typename T> [[nodiscard]] constexpr std::enable_if_t<is_enumarray<T>, bool> isNextNone(T x) noexcept {
  return static_cast<T>(to_underlying(x) + 1) == T::None;
}

} // namespace kanji_tools

#endif // KANJI_TOOLS_UTILS_ENUM_ARRAY_H
