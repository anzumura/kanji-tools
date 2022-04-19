#pragma once

#include <kanji_tools/utils/IterableEnum.h>

#include <array>
#include <iostream>
#include <map>

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
//   template<> inline constexpr auto is_enumarray<Colors>{true};
//   inline const auto AllColors{TypedEnumArray<Colors>::create("Red", "Green",
//     "Blue")};
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
//   template<> inline constexpr auto is_enumarray_with_none<Colors>{true};
//   inline const auto AllColors{TypedEnumArray<Colors>::create("Red", "Green",
//     "Blue")};
//
//   // prints all Colors including 'None'
//   for (auto c : AllColors) { std::cout << c << '\n'; }
//
// A scoped enum can't be both an 'EnumArray' and an 'EnumArrayWithNone'.

// specialize 'is_enumarray' to enable 'EnumArray'
template<typename T, std::enable_if_t<is_scoped_enum_v<T>, int> = 0>
inline constexpr auto is_enumarray{false};

// specialize 'is_enumarray_with_none' to enable 'EnumArrayWithNone'
template<typename T, std::enable_if_t<is_scoped_enum_v<T>, int> = 0>
inline constexpr auto is_enumarray_with_none{false};

template<typename T, typename _ = int>
using isEnumArray =
    std::enable_if_t<is_enumarray<T> || is_enumarray_with_none<T>, _>;

class BaseEnumArray {
public:
  virtual ~BaseEnumArray() = 0;
protected:
  static void domainError(const std::string&);
};

// 'TypedEnumArray' has a pure virtual 'toString' function holds a map from
// 'std::string' to 'Ts' used by 'fromString' methods. It also has static
// 'create', 'isCreated' and 'instance' public functions.
template<typename T, isEnumArray<T> = 0>
class TypedEnumArray : public BaseEnumArray {
public:
  // 'create' requires at least one 'name' (see comments above)
  template<typename... Names>
  [[nodiscard]] static auto create(const std::string& name, Names...);

  [[nodiscard]] static bool isCreated() noexcept { return _instance; }

  [[nodiscard]] static auto& instance() {
    if (!_instance) domainError("must call 'create' before calling 'instance'");
    return *_instance;
  }

  ~TypedEnumArray() override { _instance = nullptr; }

  [[nodiscard]] virtual const std::string& toString(T) const = 0;

  // returns 'T' instance for the given string (does not support T::None). See
  // EnumArrayWithNone form more 'fromString...' methods that support T::None.
  [[nodiscard]] T fromString(const std::string& name) const;
protected:
  TypedEnumArray() noexcept { _instance = this; }

  void insert(const std::string& name, BaseIterableEnum::Index index);
private:
  inline static constinit const TypedEnumArray<T>* _instance;

  std::map<std::string, T> _nameMap;
};

// 'IterableEnumArray' adds functionality to the iterator from 'IterableEnum'
// that is common to both derived classes ('EnumArray' and 'EnumArrayWithNone')
// and has public 'begin', 'end' and 'operator[]' functions.
template<typename T, size_t N>
class IterableEnumArray : public IterableEnum<T, N>, public TypedEnumArray<T> {
public:
  using base = IterableEnum<T, N>;

  [[nodiscard]] static auto begin() noexcept { return ConstIterator{0}; }
  [[nodiscard]] static auto end() noexcept { return ConstIterator{N}; }

  template<std::integral I> [[nodiscard]] auto operator[](I i) const {
    return static_cast<T>(base::checkIndex(i, base::IndexMsg));
  }

  class ConstIterator : public base::template Iterator<ConstIterator> {
  public:
    // base iterator implements some operations such as prefix and postfix
    // increment and decrement, operator[], +=, -=, + and -.
    using iBase = typename base::template Iterator<ConstIterator>;

    // forward iterator requirements (default ctor)

    ConstIterator() noexcept : iBase{0} {}

    // input iterator requirements (except operator->)

    [[nodiscard]] auto operator*() const {
      // exception should only happen when dereferencing 'end' since other
      // methods prevent moving out of range
      if (iBase::index() >= N)
        iBase::rangeError(
            base::IndexMsg + std::to_string(iBase::index()) + base::RangeMsg);
      return static_cast<T>(iBase::index());
    }

    // random-access iterator requirements

    using iBase::operator-;
    [[nodiscard]] auto operator-(const ConstIterator& x) const noexcept {
      return iBase::index() - x.index();
    }
  private:
    friend IterableEnumArray<T, N>; // calls private ctor

    explicit ConstIterator(BaseIterableEnum::Index index) noexcept
        : iBase{index} {}
  };
protected:
  IterableEnumArray() noexcept = default;
};

// see comments at the top of this file (EnumArray.h) for more details
template<typename T, size_t N>
class EnumArray : public IterableEnumArray<T, N> {
public:
  using base = IterableEnumArray<T, N>;

  [[nodiscard]] const std::string& toString(T x) const override {
    return _names[base::getIndex(x)];
  }
private:
  friend TypedEnumArray<T>; // 'create' calls private ctor

  explicit EnumArray(const std::string& name) { setName(name, N - 1); }

  template<typename... Names>
  EnumArray(const std::string& name, Names... args) : EnumArray{args...} {
    setName(name, N - 1 - sizeof...(args));
  }

  void setName(const std::string& name, BaseIterableEnum::Index index) {
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
    const auto i{base::getIndex(x)};
    return i < N ? _names[i] : None;
  }

  [[nodiscard]] auto fromStringAllowEmpty(const std::string& s) const {
    return s.empty() ? T::None : base::fromString(s);
  }

  [[nodiscard]] auto fromStringAllowNone(const std::string& s) const {
    return s == None ? T::None : base::fromString(s);
  }

  [[nodiscard]] auto fromStringAllowEmptyAndNone(const std::string& s) const {
    return s.empty() || s == None ? T::None : base::fromString(s);
  }
private:
  inline const static std::string None{"None"};
  friend TypedEnumArray<T>; // 'create' calls private ctor

  explicit EnumArrayWithNone(const std::string& name) { setName(name, N - 1); }

  template<typename... Names>
  EnumArrayWithNone(const std::string& name, Names... args)
      : EnumArrayWithNone{args...} {
    setName(name, N - 1 - sizeof...(args));
  }

  void setName(const std::string& name, BaseIterableEnum::Index index) {
    if (name == None) base::domainError("'None' should not be specified");
    base::insert(_names[index] = name, index);
  }

  std::array<std::string, N> _names;
};

// out of class member definitions for 'TypedEnumArray<T>'

template<typename T, isEnumArray<T> _>
template<typename... Names>
[[nodiscard]] auto TypedEnumArray<T, _>::create(
    const std::string& name, Names... args) {
  static_assert(is_enumarray<T> != is_enumarray_with_none<T>,
      "both 'is_enumarray' and 'is_enumarray_with_none' are true");
  if (_instance) domainError("'create' should only be called once");
  if constexpr (is_enumarray<T>)
    return EnumArray<T, sizeof...(args) + 1>{name, args...};
  else {
    // Make sure the scoped enum 'T' has a value of None that is just past the
    // set of string values provided - this will help ensure that any changes to
    // the enum must also be made to the call to 'create' and vice versa.
    static_assert(to_enum<T>(sizeof...(Names) + 1) == T::None);
    return EnumArrayWithNone<T, sizeof...(args) + 1>{name, args...};
  }
}

template<typename T, isEnumArray<T> _>
T TypedEnumArray<T, _>::fromString(const std::string& name) const {
  const auto i{_nameMap.find(name)};
  if (i == _nameMap.end()) domainError("name '" + name + "' not found");
  return i->second;
}

template<typename T, isEnumArray<T> _>
void TypedEnumArray<T, _>::insert(
    const std::string& name, BaseIterableEnum::Index index) {
  if (!_nameMap.emplace(name, static_cast<T>(index)).second)
    domainError("duplicate name '" + name + "'");
}

// below are some global functions that are enabled for enums types that have
// instances of 'EnumArray' or 'EnumArrayWithNone'

template<typename T>
[[nodiscard]] isEnumArray<T, const std::string&> toString(T x) {
  return TypedEnumArray<T>::instance().toString(x);
}

template<typename T>
isEnumArray<T, std::ostream&> operator<<(std::ostream& os, T x) {
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
  return to_enum<T>(to_underlying(x) + 1) == T::None;
}

} // namespace kanji_tools
