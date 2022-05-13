#pragma once

#include <kanji_tools/utils/EnumContainer.h>

#include <array>
#include <iostream>
#include <map>
#include <stdexcept>

namespace kanji_tools {

// EnumList is a helper class for scoped enums that have contiguous values
// starting at zero. It provides 'size', 'operator[]' and 'fromString' methods
// as well as 'begin' and 'end' methods for range based iteration over the enum
// values. There is also a 'toString' method that is used by out-of-class
// 'toString' and ostream 'operator<<' functions.
//
// In order to enable this functionality 'is_enumlist' must be set to 'true'
// and an EnumList instance must be created with string values in the same order
// as the scoped enum values.
//
// Here's an example of how to create (and use) an EnumList:
//
//   enum class Colors : BaseEnum::Size { Red, Green, Blue };
//   template<> inline constexpr auto is_enumlist<Colors>{true};
//   inline const auto AllColors{BaseEnumList<Colors>::create("Red", "Green",
//     "Blue")};
//
//   for (auto c : AllColors) { std::cout << c << '\n'; }
//
// If the enum has a final value of 'None' then extra functionality (and
// compile checks) can be enabled by setting 'is_enumlist_with_none' instead.
// Setting this bool enables 'hasValue', 'operator!' and 'isNextNone' global
// functions based on T::None. 'None' should not be passed to 'create', instead
// there's a static_assert to check that T::None is the next value after the
// list of strings (helps keep the enum and the strings in sync).
//
// Here's an example of how to create (and use) an EnumList with 'None':
//
//   enum class Colors : BaseEnum::Size { Red, Green, Blue, None };
//   template<> inline constexpr auto is_enumlist_with_none<Colors>{true};
//   inline const auto AllColors{BaseEnumList<Colors>::create("Red", "Green",
//     "Blue")};
//
//   // prints all Colors including 'None'
//   for (auto c : AllColors) { std::cout << c << '\n'; }
//
// A scoped enum can't be both an 'EnumList' and an 'EnumListWithNone'.

// specialize 'is_enumlist' to enable 'EnumList'
template<typename T, std::enable_if_t<is_scoped_enum_v<T>, int> = 0>
inline constexpr auto is_enumlist{false};

// specialize 'is_enumlist_with_none' to enable 'EnumListWithNone'
template<typename T, std::enable_if_t<is_scoped_enum_v<T>, int> = 0>
inline constexpr auto is_enumlist_with_none{false};

template<typename T, typename _ = int>
using isEnumList =
    std::enable_if_t<is_enumlist<T> || is_enumlist_with_none<T>, _>;

// 'BaseEnumList' has a pure virtual 'toString' function holds a map from
// 'String' to 'Ts' used by 'fromString' methods. It also has static 'create',
// 'isCreated' and 'instance' public functions.
template<typename T, isEnumList<T> = 0> class BaseEnumList {
public:
  // 'create' requires at least one 'name' (see comments above)
  template<typename... Names>
  [[nodiscard]] static auto create(const String& name, Names...);

  [[nodiscard]] static bool isCreated() noexcept { return _instance; }

  [[nodiscard]] static auto& instance() {
    if (!_instance) domainError("must call 'create' before calling 'instance'");
    return *_instance;
  }

  virtual ~BaseEnumList() { _instance = nullptr; }

  [[nodiscard]] virtual const String& toString(T) const = 0;

  // returns 'T' instance for the given string (does not support T::None). See
  // EnumListWithNone form more 'fromString...' methods that support T::None.
  [[nodiscard]] T fromString(const String& name) const;
protected:
  static void domainError(const String& msg) { throw std::domain_error{msg}; }

  BaseEnumList() noexcept { _instance = this; }

  void insert(const String& name, EnumContainer::Size index);
private:
  inline static constinit const BaseEnumList<T>* _instance;

  std::map<String, T> _nameMap;
};

// 'EnumListContainer' provides common functionality for derived classes
// including 'begin', 'end', 'operator[]' and 'toString'. Template args are:
// - T:     scoped enum type
// - N:     number of enum values (used for interating and index checking)
// - Names: number of strings in '_names' array (same as N by default, but set
//          to 'N - 1' for EnumListWithNone since 'None' isn't stored)
template<typename T, EnumContainer::Size N, EnumContainer::Size Names = N>
class EnumListContainer : public TypedEnumContainer<T, N>,
                          public BaseEnumList<T> {
public:
  using base = TypedEnumContainer<T, N>;

  [[nodiscard]] static auto begin() noexcept { return ConstIterator{0}; }
  [[nodiscard]] static auto end() noexcept { return ConstIterator{N}; }

  template<std::integral I> [[nodiscard]] auto operator[](I i) const {
    return to_enum<T>(base::checkIndex(i, base::IndexMsg));
  }

  [[nodiscard]] const String& toString(T x) const override {
    return _names[base::getIndex(x)];
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
      return to_enum<T>(iBase::index());
    }

    // random-access iterator requirements

    using iBase::operator-;
    [[nodiscard]] auto operator-(const ConstIterator& x) const noexcept {
      return iBase::index() - x.index();
    }
  private:
    friend EnumListContainer<T, N, Names>; // calls private ctor

    explicit ConstIterator(EnumContainer::Size index) noexcept : iBase{index} {}
  };
protected:
  EnumListContainer() noexcept = default;

  void setName(const String& name, EnumContainer::Size index) {
    BaseEnumList<T>::insert(_names[index] = name, index);
  }
private:
  std::array<String, Names> _names;
};

// see comments at the top of this file (EnumList.h) for more details
template<typename T, EnumContainer::Size N>
class EnumList : public EnumListContainer<T, N> {
public:
  using base = EnumListContainer<T, N, N>;
private:
  friend BaseEnumList<T>; // 'create' calls private ctor

  explicit EnumList(const String& name) { base::setName(name, N - 1); }

  template<typename... Names>
  explicit EnumList(const String& name, Names... args) : EnumList{args...} {
    base::setName(name, N - 1 - sizeof...(args));
  }
};

// Base 'EnumListContainer' has size 'N + 1' to account for final 'None' when
// iterating. A string value for 'None' is not stored in '_names' or base class
// '_nameMap' for safety (see private 'setName') and to support the special
// handling in 'fromString' with 'allowEmptyAsNone'.
template<typename T, EnumContainer::Size N>
class EnumListWithNone : public EnumListContainer<T, N + 1, N> {
public:
  using base = EnumListContainer<T, N + 1, N>;

  [[nodiscard]] const String& toString(T x) const override {
    return x == T::None ? None : base::toString(x);
  }

  [[nodiscard]] auto fromStringAllowEmpty(const String& s) const {
    return s.empty() ? T::None : base::fromString(s);
  }

  [[nodiscard]] auto fromStringAllowNone(const String& s) const {
    return s == None ? T::None : base::fromString(s);
  }

  [[nodiscard]] auto fromStringAllowEmptyAndNone(const String& s) const {
    return s.empty() || s == None ? T::None : base::fromString(s);
  }
private:
  inline const static String None{"None"};
  friend BaseEnumList<T>; // 'create' calls private ctor

  explicit EnumListWithNone(const String& name) { setName(name, N - 1); }

  template<typename... Names>
  explicit EnumListWithNone(const String& name, Names... args)
      : EnumListWithNone{args...} {
    setName(name, N - 1 - sizeof...(args));
  }

  void setName(const String& name, EnumContainer::Size index) {
    if (name == None) base::domainError("'None' should not be specified");
    base::setName(name, index);
  }
};

// out of class member definitions for 'BaseEnumList<T>'

template<typename T, isEnumList<T> U>
template<typename... Names>
auto BaseEnumList<T, U>::create(const String& name, Names... args) {
  static_assert(is_enumlist<T> != is_enumlist_with_none<T>,
      "both 'is_enumlist' and 'is_enumlist_with_none' are true");
  if (_instance) domainError("'create' should only be called once");
  if constexpr (is_enumlist<T>)
    return EnumList<T, sizeof...(args) + 1>{name, args...};
  else {
    // Make sure the scoped enum 'T' has a value of None that is just past the
    // set of string values provided - this will help ensure that any changes to
    // the enum must also be made to the call to 'create' and vice versa.
    static_assert(to_enum<T>(sizeof...(Names) + 1) == T::None);
    return EnumListWithNone<T, sizeof...(args) + 1>{name, args...};
  }
}

template<typename T, isEnumList<T> U>
T BaseEnumList<T, U>::fromString(const String& name) const {
  const auto i{_nameMap.find(name)};
  if (i == _nameMap.end()) domainError("name '" + name + "' not found");
  return i->second;
}

template<typename T, isEnumList<T> U>
void BaseEnumList<T, U>::insert(const String& name, EnumContainer::Size index) {
  if (!_nameMap.emplace(name, to_enum<T>(index)).second)
    domainError("duplicate name '" + name + "'");
}

// below are some global functions that are enabled for enums types that have
// instances of 'EnumList' or 'EnumListWithNone'

template<typename T> [[nodiscard]] isEnumList<T, const String&> toString(T x) {
  return BaseEnumList<T>::instance().toString(x);
}

template<typename T>
isEnumList<T, std::ostream&> operator<<(std::ostream& os, T x) {
  return os << toString(x);
}

template<typename T>
using isEnumListWithNone = std::enable_if_t<is_enumlist_with_none<T>, bool>;

template<typename T>
[[nodiscard]] constexpr isEnumListWithNone<T> hasValue(T x) noexcept {
  return x != T::None;
}

template<typename T>
[[nodiscard]] constexpr isEnumListWithNone<T> operator!(T x) noexcept {
  return !hasValue(x);
}

// 'isNextNone' returns true if the next value after 'x' is T::None
template<typename T>
[[nodiscard]] constexpr isEnumListWithNone<T> isNextNone(T x) noexcept {
  return to_enum<T>(to_underlying(x) + 1) == T::None;
}

} // namespace kanji_tools
