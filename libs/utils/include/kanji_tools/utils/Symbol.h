#pragma once

#include <kanji_tools/utils/String.h>

#include <iostream>
#include <map>
#include <numeric>
#include <vector>

namespace kanji_tools {

//! \lib_utils{Symbol} non-templated base class for Symbol
class BaseSymbol {
public:
  using Id = uint16_t;

  //! maximum number of unique symbols that can be created per 'type'
  static constexpr auto Max{std::numeric_limits<Id>::max() - 1};

  //! '0' is used as the 'id' for 'empty' symbols (non-empty start at '1')
  [[nodiscard]] constexpr auto id() const noexcept { return _id; }

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

//! \lib_utils{Symbol} class that can be used instead of #String to save memory
//!
//! Symbol incurs a small performance hit when creating/looking up a value, but
//! can save significant memory when used as a member of a class that has many
//! instances and the member doesn't have many different values. Some good
//! examples would be Unicode block or version names (see Ucd.h for examples).
//!
//! Currently up to ~65K unique symbols per type can be added (an exception is
//! thrown if the limit is exceeded). If more than 65K values are needed then
//! Symbol was probably not the right design choice in the first place.
//!
//! Classes should derive from **Symbol<T>** and define **Type**, for example:
//! \code
//! class TestSymbol : public Symbol<TestSymbol> {
//! public:
//!   inline static const String Type{"TestSymbol"};
//!   using Symbol::Symbol;
//! };
//! \endcode
//!
//! \tparam T derived class (see sample code above)
template<typename T> class Symbol : public BaseSymbol {
public:
  [[nodiscard]] static const String& type() noexcept { return T::Type; }

  //! returns total unique symbols created (not including 'empty' symbol)
  [[nodiscard]] static auto size() noexcept { return _list.size(); }

  //! returns true if a symbol exists for the given (non-empty) name
  [[nodiscard]] static auto exists(const String& name) {
    return !name.empty() && _map.contains(name);
  }

  constexpr Symbol() noexcept : BaseSymbol{0} {}
  explicit Symbol(const String& name) : BaseSymbol{type(), name, _map, _list} {}

  [[nodiscard]] auto& name() const {
    return id() ? *_list.at(id() - 1) : EmptyString;
  }

  [[nodiscard]] constexpr auto operator==(const Symbol& x) const noexcept {
    return id() == x.id();
  }

  [[nodiscard]] constexpr auto operator!=(const Symbol& x) const noexcept {
    return !operator==(x);
  }
private:
  inline static Map _map;
  inline static List _list;
};

template<typename T>
std::ostream& operator<<(std::ostream& os, const Symbol<T>& s) {
  return os << s.name();
}

} // namespace kanji_tools
