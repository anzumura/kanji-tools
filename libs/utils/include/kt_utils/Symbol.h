#pragma once

#include <kt_utils/String.h>

#include <iostream>
#include <map>
#include <numeric>
#include <vector>

namespace kanji_tools { /// \utils_group{Symbol}
/// Symbol class that can be used to save memory instead of using #String

/// non-templated base class for Symbol \utils{Symbol}
class BaseSymbol {
public:
  using Id = uint16_t; ///< type for id

  /// maximum number of unique symbols that can be created per symbol 'type'
  static constexpr auto Max{std::numeric_limits<Id>::max() - 1};

  /// return the id, an empty Symbol has an id of `0`
  [[nodiscard]] constexpr auto id() const noexcept { return _id; }

  /// return true if this Symbol is non-empty
  [[nodiscard]] explicit constexpr operator bool() const { return _id; }
protected:
  using Map = std::map<String, Id>;
  using List = std::vector<const String*>;

  constexpr explicit BaseSymbol(Id id) noexcept : _id{id} {}
  BaseSymbol(const String& type, const String& name, Map&, List&);
private:
  [[nodiscard]] static Id getId(
      const String& type, const String& name, Map&, List&);

  const Id _id;
};

/// class can be used instead of #String to save memory \utils{Symbol}
///
/// Symbol incurs a small performance hit when creating/looking up a value, but
/// can save significant memory when used as a member of a class that has many
/// instances and the member doesn't have many different values. Some good
/// examples would be Unicode block or version names (see Ucd.h for examples).
///
/// Currently up to ~65K unique symbols per type can be added (an exception is
/// thrown if the limit is exceeded). If more than 65K values are needed then
/// Symbol was probably not the right design choice in the first place.
///
/// Classes should derive from **Symbol<T>** and define **Type**, for example:
/// \code
///   class TestSymbol final : public Symbol<TestSymbol> {
///   public:
///     inline static const String Type{"TestSymbol"};
///     using Symbol::Symbol;
///   };
/// \endcode
///
/// \tparam T derived class (see sample code above)
template<typename T> class Symbol : public BaseSymbol {
public:
  /// return the type name
  [[nodiscard]] static const String& type() noexcept { return T::Type; }

  /// return total unique (non-empty) Symbols created
  [[nodiscard]] static auto size() noexcept { return _list.size(); }

  /// return true if a Symbol exists for the given (non-empty) `name`
  [[nodiscard]] static auto exists(const String& name) {
    return !name.empty() && _map.contains(name);
  }

  /// default ctor, creates an empty Symbol with id `0`
  constexpr Symbol() noexcept : BaseSymbol{0} {}

  /// create a Symbol for the given String
  /// \param name the name of the Symbol
  /// \throw DomainError if `name` would result in more than #Max Symbols
  explicit Symbol(const String& name) : BaseSymbol{type(), name, _map, _list} {}

  /// return the name or empty String if id is `0`
  [[nodiscard]] auto& name() const {
    return id() ? *_list.at(id() - 1) : emptyString();
  }

  /// equal operator
  [[nodiscard]] constexpr auto operator==(const Symbol& x) const noexcept {
    return id() == x.id();
  }

  /// not-equal operator
  [[nodiscard]] constexpr auto operator!=(const Symbol& x) const noexcept {
    return !operator==(x);
  }
private:
  inline static Map _map;
  inline static List _list;
};

/// write Symbol::name() to `os`
template<typename T>
std::ostream& operator<<(std::ostream& os, const Symbol<T>& s) {
  return os << s.name();
}

/// \end_group
} // namespace kanji_tools
