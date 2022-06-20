#pragma once

#include <kt_utils/EnumContainer.h>
#include <kt_utils/Exception.h>

#include <array>
#include <iostream>
#include <map>

namespace kanji_tools { /// \utils_group{EnumList}
/// classes that provide conversion to/from string as well as iteration over the
/// values of a scoped enum

/// specialize to enable an enum to be used in EnumList class
template<scoped_enum T> constexpr auto is_enumlist{false};

/// specialize to enable an enum to be used in EnumListWithNone class
template<scoped_enum T> constexpr auto is_enumlist_with_none{false};

template<typename T>
concept enumlist = is_enumlist<T>;

template<typename T>
concept enumlist_with_none = is_enumlist_with_none<T>;

template<typename T>
concept base_enumlist = enumlist<T> || enumlist_with_none<T>;

/// base class with pure virtual toString() method as well as fromString() and
/// static create() and instance() methods \utils{EnumList}
///
/// The concrete EnumList and EnumListWithNone classes provide more methods such
/// as size(), operator[]() as well as begin() and end() for iterating over the
/// enum values. Once an instance of one of the derived classes has been created
/// via the create() method then global toString() and operator<<() functions
/// can also be used for `T` (the enum).
///
/// If the enum has a final 'None' value then extra functionality (and compile
/// checks) can be enabled by specializing #is_enumlist_with_none instead of
/// #is_enumlist. Setting this bool also enables hasValue(), operator!() and
/// isNextNone() global functions based on T::None. "None" string shouldn't be
/// passed to create(), instead there's a static_assert to check that T::None is
/// the next value after the list of strings (this helps keep the enum and the
/// strings in sync).
///
/// A scoped enum can't be enabled for both EnumList and EnumListWithNone.
///
/// \tparam T scoped enum with contiguous values starting at zero
template<base_enumlist T> class BaseEnumList {
public:
  /// create an instance of EnumList or EnumListWithNone (depending on which
  /// bool has been specialized for `T`) from one or more String names
  /// \throw DomainError if an instance has already been created or if a name is
  ///     used more than once or if "None" is specified for an EnumListWithNone
  template<typename... Names>
  [[nodiscard]] static auto create(const String& name, Names...);

  /// return true if an instance of BaseEnumList<T> has been created
  [[nodiscard]] static bool isCreated() noexcept { return _instance; }

  /// return static instance, create() must be called before using this method
  /// \throw DomainError if instance hasn't been created
  [[nodiscard]] static auto& instance() {
    if (!_instance) domainError("must call 'create' before calling 'instance'");
    return *_instance;
  }

  /// dtor sets instance back to nullptr
  virtual ~BaseEnumList() { _instance = nullptr; }

  /// implemented by derived EnumListContainer class
  [[nodiscard]] virtual const String& toString(T) const = 0;

  /// return `T` instance for `name` (see EnumListWithNone for more 'fromString'
  /// methods that support T::None)
  /// \throw DomainError if `name` doesn't map to an enum value of `T`
  [[nodiscard]] T fromString(const String& name) const;
protected:
  static void domainError(const String& msg) { throw DomainError{msg}; }

  BaseEnumList() noexcept { _instance = this; }

  void insert(const String& name, Enum::Size index);
private:
  inline static constinit const BaseEnumList<T>* _instance;

  std::map<String, T> _nameMap;
};

/// provide common functionality for derived classes including begin(), end(),
/// operator[]() and toString() \utils{EnumList}
/// \tparam T scoped enum with contiguous values starting at zero
/// \tparam N number of enum values used for iterating and index checking
/// \tparam Names number of string names (same as N by default)
template<base_enumlist T, Enum::Size N, Enum::Size Names = N>
class EnumListContainer : public EnumContainer<T, N>, public BaseEnumList<T> {
public:
  using base = EnumContainer<T, N>;

  /// return ConstIterator pointing at the first enum value
  [[nodiscard]] static auto begin() noexcept { return ConstIterator{0}; }

  /// return ConstIterator pointing at 'one-past' the last enum value
  [[nodiscard]] static auto end() noexcept { return ConstIterator{N}; }

  /// return enum value at position `i`
  /// \tparam I must be in integral type
  /// \throw RangeError if `i` is negative or beyond the final enum value
  template<std::integral I> [[nodiscard]] auto operator[](I i) const {
    return to_enum<T>(base::checkIndex(i, base::IndexMsg));
  }

  /// return String name for enum value `x`
  /// \throw RangeError if `x` is out of range (can only happen if a bad value
  ///     is cast to the enum type)
  [[nodiscard]] const String& toString(T x) const override {
    return _names[base::getIndex(x)];
  }

  /// iterator for looping over values of `T` (the enum) \utils{EnumList}
  class ConstIterator final : public base::template Iterator<ConstIterator> {
  public:
    using iBase = typename base::template Iterator<ConstIterator>;
    using iBase::operator-, iBase::index, iBase::rangeError;

    /// default ctor sets location to first value (forward iterator)
    ConstIterator() noexcept : iBase{0} {}

    /// return value at current location (input iterator)
    /// \throw RangeError if location is invalid, i.e., at 'end' location
    [[nodiscard]] auto operator*() const {
      if (index() >= N)
        rangeError(base::IndexMsg + std::to_string(index()) + base::RangeMsg);
      return to_enum<T>(index());
    }

    /// return difference between iterators (random-access iterator)
    [[nodiscard]] auto operator-(const ConstIterator& x) const noexcept {
      return index() - x.index();
    }
  private:
    friend EnumListContainer<T, N, Names>; // calls private ctor

    explicit ConstIterator(Enum::Size i) noexcept : iBase{i} {}
  };
protected:
  EnumListContainer() noexcept = default;

  void setName(const String& name, Enum::Size index) {
    BaseEnumList<T>::insert(_names[index] = name, index);
  }
private:
  std::array<String, Names> _names;
};

/// provide iteration and conversion to/from String for enums \utils{EnumList}
///
/// Here's an example of how to create and use EnumList:
/// \code
///   // must have underlying type Enum::Size and values starting at 0
///   enum class Colors : Enum::Size { Red, Green, Blue };
///
///   // need to specialize is_enumlist before calling 'create'
///   template<> inline constexpr auto is_enumlist<Colors>{true};
///
///   // call 'create' with strings in the same order as the enum values
///   inline const auto AllColors{BaseEnumList<Colors>::create("Red", "Green",
///     "Blue")};
///
///   // print all Colors
///   for (auto c : AllColors) { std::cout << c << '\n'; }
/// \endcode
///
/// \tparam T scoped enum with contiguous values starting at zero
/// \tparam N number of enum values
template<enumlist T, Enum::Size N>
class EnumList final : public EnumListContainer<T, N> {
private:
  using base = EnumListContainer<T, N, N>;

  friend BaseEnumList<T>; // 'create' calls private ctor

  explicit EnumList(const String& name) { base::setName(name, N - 1); }

  template<typename... Names>
  explicit EnumList(const String& name, Names... args) : EnumList{args...} {
    base::setName(name, N - 1 - sizeof...(args));
  }
};

/// provide iteration and conversion to/from String for enums that have a final
/// 'None' value \utils{EnumList}
///
/// The base class is instantiated with size `N + 1` to account T::None when
/// iterating, but is also passed `N` for the size of `_names` since a "None"
/// String is not stored for safety, see setName(), as well as to support the
/// special handling in fromString() with `allowEmptyAsNone`.
///
/// Here's an example of how to create and use EnumListWithNone:
/// \code
///   // must have underlying type Enum::Size, start at 0 and final value 'None'
///   enum class Colors : Enum::Size { Red, Green, Blue, None };
///
///   // need to specialize is_enumlist_with_none before calling 'create'
///   template<> inline constexpr auto is_enumlist_with_none<Colors>{true};
///
///   // call 'create' with strings in the same order as the enum values and
///   // don't specify "None"
///   inline const auto AllColors{BaseEnumList<Colors>::create("Red", "Green",
///     "Blue")};
///
///   // prints all Colors including final "None"
///   for (auto c : AllColors) { std::cout << c << '\n'; }
/// \endcode
/// \tparam T scoped enum with contiguous values starting at zero
/// \tparam N number of enum values (not including final 'None')
template<enumlist_with_none T, Enum::Size N>
class EnumListWithNone final : public EnumListContainer<T, N + 1, N> {
public:
  using base = EnumListContainer<T, N + 1, N>;

  /// return String name for enum value `x` including support for T::None
  /// \throw RangeError if `x` is out of range (can only happen if a bad value
  ///     is cast to the enum type)
  [[nodiscard]] const String& toString(T x) const final {
    return x == T::None ? None : base::toString(x);
  }

  /// return enum value for `name`, empty `name` returns T::None
  /// \throw DomainError if `name` doesn't map to an enum value of `T`
  [[nodiscard]] auto fromStringAllowEmpty(const String& name) const {
    return name.empty() ? T::None : base::fromString(name);
  }

  /// return enum value for `name`, allows "None" to return T::None
  /// \throw DomainError if `name` doesn't map to an enum value of `T`
  [[nodiscard]] auto fromStringAllowNone(const String& name) const {
    return name == None ? T::None : base::fromString(name);
  }

  /// return enum value for `name`, allows empty or "None" to return T::None
  /// \throw DomainError if `name` doesn't map to an enum value of `T`
  [[nodiscard]] auto fromStringAllowEmptyAndNone(const String& name) const {
    return name.empty() || name == None ? T::None : base::fromString(name);
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

  void setName(const String& name, Enum::Size index) {
    if (name == None) base::domainError("'None' should not be specified");
    base::setName(name, index);
  }
};

// out-of-class member definitions for BaseEnumList<T>

template<base_enumlist T>
template<typename... Names>
auto BaseEnumList<T>::create(const String& name, Names... args) {
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

template<base_enumlist T>
T BaseEnumList<T>::fromString(const String& name) const {
  const auto i{_nameMap.find(name)};
  if (i == _nameMap.end()) domainError("name '" + name + "' not found");
  return i->second;
}

template<base_enumlist T>
void BaseEnumList<T>::insert(const String& name, Enum::Size index) {
  if (!_nameMap.emplace(name, to_enum<T>(index)).second)
    domainError("duplicate name '" + name + "'");
}

// below are some global functions that are enabled for enums types that have
// instances of 'EnumList' or 'EnumListWithNone'

/// return String name for `x`
template<base_enumlist T> [[nodiscard]] const String& toString(T x) {
  return BaseEnumList<T>::instance().toString(x);
}

/// write String name of `x` to `os`
template<base_enumlist T> std::ostream& operator<<(std::ostream& os, T x) {
  return os << toString(x);
}

/// return true if `x` is not T::None
template<enumlist_with_none T>
[[nodiscard]] constexpr auto hasValue(T x) noexcept {
  return x != T::None;
}

/// return true if `x` is T::None
template<enumlist_with_none T>
[[nodiscard]] constexpr auto operator!(T x) noexcept {
  return !hasValue(x);
}

/// return true if the next value after `x` is T::None
template<enumlist_with_none T>
[[nodiscard]] constexpr auto isNextNone(T x) noexcept {
  return to_enum<T>(to_underlying(x) + 1) == T::None;
}

/// \end_group
} // namespace kanji_tools
