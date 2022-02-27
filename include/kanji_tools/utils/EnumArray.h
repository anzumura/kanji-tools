#pragma once

#include <kanji_tools/utils/EnumTraits.h>

#include <array>
#include <compare>
#include <iostream>
#include <map>
#include <string>

namespace kanji_tools {

// EnumArray is a helper class for scoped enums that have contiguous values
// starting at zero. It provides 'size', 'operator[]' and 'fromString' methods
// as well as 'begin' and 'end' methods for range based iteration over the enum
// values. There is also a 'toString' method that is used by out-of-class
// 'toString' and ostream 'operator<<' functions.
//
// In order to enable this functionality 'is_enumarray' must be set to 'true'
// and an EnumArray instance must be created with string values in the same
// order as the scoped enum values.
//
// Here's an example of how to create (and use) an EnumArray:
//
//   enum class Colors { Red, Green, Blue };
//   template<> inline constexpr bool is_enumarray<Colors> = true;
//   inline const auto AllColors = BaseEnumArray<Colors>::create("Red", "Green",
//     "Blue");
//
//   for (auto c : AllColors) { std::cout << c << '\n'; }
//
// If the enum has a final value of 'None' then extra functionality (and
// compile checks) can be enabled by setting 'is_enumarray_with_none' instead.
// Setting this bool enables 'hasValue', 'operator!' and 'isNextNone' global
// functions based on T::None. 'None' should not be passed to 'create', instead
// there's a static_assert to check that T::None is the next value after the
// list of strings (helps keep the enum and the strings in sync).
//
// Here's an example of how to create (and use) an EnumArray with 'None':
//
//   enum class Colors { Red, Green, Blue, None };
//   template<> inline constexpr bool is_enumarray_with_none<Colors> = true;
//   inline const auto AllColors = BaseEnumArray<Colors>::create("Red", "Green",
//     "Blue");
//
//   // prints all Colors including 'None'
//   for (auto c : AllColors) { std::cout << c << '\n'; }
//
// A scoped enum can't be both an 'EnumArray' and an 'EnumArrayWithNone'.

// specialize 'is_enumarray' to enable 'EnumArray'
template<typename T, std::enable_if_t<is_scoped_enum_v<T>, int> = 0>
inline constexpr bool is_enumarray = false;

// specialize 'is_enumarray_with_none' to enable 'EnumArrayWithNone'
template<typename T, std::enable_if_t<is_scoped_enum_v<T>, int> = 0>
inline constexpr bool is_enumarray_with_none = false;

template<typename T, typename _ = int>
using isBaseEnumArray =
  std::enable_if_t<is_enumarray<T> || is_enumarray_with_none<T>, _>;

template<typename T, isBaseEnumArray<T> = 0> class BaseEnumArray {
public:
  // 'create' requires at least one 'name' (see comments above)
  template<typename... Names>
  [[nodiscard]] static auto create(const std::string& name, Names...);

  [[nodiscard]] static auto& instance() {
    if (!_instance)
      throw std::domain_error("must call 'create' before calling 'instance'");
    return *_instance;
  }

  [[nodiscard]] static auto isCreated() noexcept {
    return _instance != nullptr;
  }

  virtual ~BaseEnumArray() { _instance = nullptr; }

  BaseEnumArray(const BaseEnumArray&) = delete;
  BaseEnumArray& operator=(const BaseEnumArray&) = delete;

  [[nodiscard]] virtual const std::string& toString(T) const = 0;
protected:
  BaseEnumArray() noexcept { _instance = this; }

  [[nodiscard]] auto find(const std::string& name) const {
    const auto i = _nameMap.find(name);
    if (i == _nameMap.end())
      throw std::domain_error("name '" + name + "' not found");
    return i->second;
  }

  void insert(const std::string& name, size_t index) {
    if (!_nameMap.emplace(name, static_cast<T>(index)).second)
      throw std::domain_error("duplicate name '" + name + "'");
  }

  std::map<std::string, T> _nameMap;
  inline static const BaseEnumArray<T>* _instance = nullptr;
};

template<typename T, size_t N>
class IterableEnumArray : public BaseEnumArray<T> {
public:
  // Random access iterator for looping over all values of T (the scoped enum).
  // This iterator does not allow modifying entries.
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
      if (_index >= N) throw std::out_of_range("can't increment past end");
      ++_index;
      return *this;
    }
    auto operator++(int) {
      Iterator x = *this;
      ++*this;
      return x;
    }

    // operator<=> enables == and != needed for 'input interator' and <, >, <=
    // and >= needed for 'random access iterator'
    [[nodiscard]] auto operator<=>(const Iterator& x) const noexcept = default;

    // input iterator requirements (except operator->)
    [[nodiscard]] auto operator*() const {
      // exception should only happen when dereferencing 'end' since other
      // methods prevent moving out of range
      if (_index >= N)
        throw std::out_of_range("index '" + std::to_string(_index) +
                                "' is out of range");
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
      if (_index + offset < 0 || _index + offset > N)
        throw std::out_of_range("can't increment past end");
      _index += offset;
      return *this;
    }
    auto& operator-=(difference_type offset) { return *this += -offset; }
    [[nodiscard]] auto operator[](difference_type offset) const {
      return *(*this + offset);
    }
    [[nodiscard]] auto operator+(difference_type offset) const {
      Iterator x = *this;
      return x += offset;
    }
    [[nodiscard]] auto operator-(difference_type offset) const {
      Iterator x = *this;
      return x -= offset;
    }
    [[nodiscard]] auto operator-(const Iterator& x) const noexcept {
      return _index - x._index;
    }
  private:
    size_t _index = 0;
  };

  [[nodiscard]] static auto begin() noexcept { return Iterator(0); }
  [[nodiscard]] static auto end() noexcept { return Iterator(N); }

  [[nodiscard]] static constexpr size_t size() noexcept { return N; }

  [[nodiscard]] auto operator[](size_t i) const {
    if (i >= N)
      throw std::out_of_range("index '" + std::to_string(i) +
                              "' is out of range");
    return static_cast<T>(i);
  }
protected:
  [[nodiscard]] static auto getIndex(T x) {
    size_t i = to_underlying(x);
    if (i >= N)
      throw std::out_of_range("enum '" + std::to_string(i) +
                              "' is out of range");
    return i;
  }
};

template<typename T, size_t N>
class EnumArray : public IterableEnumArray<T, N> {
public:
  using base = IterableEnumArray<T, N>;

  [[nodiscard]] const std::string& toString(T x) const override {
    return _names[base::getIndex(x)];
  }

  [[nodiscard]] auto fromString(const std::string& s) const {
    return base::find(s);
  }
private:
  friend BaseEnumArray<T>; // 'create' calls private constructor

  EnumArray(const std::string& name) { setName(name, N - 1); }

  template<typename... Names>
  EnumArray(const std::string& name, Names... args) : EnumArray(args...) {
    setName(name, N - 1 - sizeof...(args));
  }

  void setName(const std::string& name, size_t index) {
    base::insert(_names[index] = name, index);
  }

  std::array<std::string, N> _names;
};

// Base 'IterableEnumArray' has size 'N + 1' to account for the final 'None'
// value. A string value for 'None' is not stored in '_names' or base class
// '_nameMap' for safety (see private 'setName') as well as to support the
// special handling in 'fromString' with 'allowEmptyAsNone'.
template<typename T, size_t N>
class EnumArrayWithNone : public IterableEnumArray<T, N + 1> {
public:
  using base = IterableEnumArray<T, N + 1>;

  [[nodiscard]] const std::string& toString(T x) const override {
    size_t i = base::getIndex(x);
    return i < N ? _names[i] : None;
  }

  [[nodiscard]] auto fromString(const std::string& s,
                                bool allowEmptyAsNone = false) const {
    return allowEmptyAsNone && s.empty() || s == None ? T::None : base::find(s);
  }
private:
  inline const static std::string None = "None";
  friend BaseEnumArray<T>; // 'create' calls private constructor

  EnumArrayWithNone(const std::string& name) { setName(name, N - 1); }

  template<typename... Names>
  EnumArrayWithNone(const std::string& name, Names... args)
      : EnumArrayWithNone(args...) {
    setName(name, N - 1 - sizeof...(args));
  }

  void setName(const std::string& name, size_t index) {
    if (name == None) throw std::domain_error("'None' should not be specified");
    base::insert(_names[index] = name, index);
  }

  std::array<std::string, N> _names;
};

template<typename T, isBaseEnumArray<T> _>
template<typename... Names>
[[nodiscard]] auto BaseEnumArray<T, _>::create(const std::string& name,
                                               Names... args) {
  static_assert(is_enumarray<T> != is_enumarray_with_none<T>,
                "both 'is_enumarray' and 'is_enumarray_with_none' are true");
  if (_instance) throw std::domain_error("'create' should only be called once");
  if constexpr (is_enumarray<T>)
    return EnumArray<T, sizeof...(args) + 1>(name, args...);
  else {
    // Make sure the scoped enum 'T' has a value of None that is just past the
    // set of string values provided - this will help ensure that any changes to
    // the enum must also be made to the call to 'create' and vice versa.
    static_assert(static_cast<T>(sizeof...(Names) + 1) == T::None);
    return EnumArrayWithNone<T, sizeof...(args) + 1>(name, args...);
  }
}

template<typename T>
[[nodiscard]] isBaseEnumArray<T, const std::string&> toString(T x) {
  return BaseEnumArray<T>::instance().toString(x);
}

template<typename T>
isBaseEnumArray<T, std::ostream&> operator<<(std::ostream& os, T x) {
  return os << toString(x);
}

template<typename T>
using isEnumArrayWithNone = std::enable_if_t<is_enumarray_with_none<T>, bool>;

template<typename T>
[[nodiscard]] constexpr isEnumArrayWithNone<T> hasValue(T x) noexcept {
  return x != T::None;
}

template<typename T>
[[nodiscard]] constexpr isEnumArrayWithNone<T> operator!(T x) noexcept {
  return !hasValue(x);
}

// 'isNextNone' returns true if the next value after 'x' is T::None
template<typename T>
[[nodiscard]] constexpr isEnumArrayWithNone<T> isNextNone(T x) noexcept {
  return static_cast<T>(to_underlying(x) + 1) == T::None;
}

} // namespace kanji_tools
