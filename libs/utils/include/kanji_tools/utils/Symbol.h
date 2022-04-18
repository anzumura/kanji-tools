#pragma once

#include <kanji_tools/utils/Utils.h>

#include <iostream>
#include <map>
#include <numeric>
#include <vector>

namespace kanji_tools {

// 'Symbol' can be used instead of 'std::string' to save memory at the cost of
// a small performance hit when creating or looking up a value. A potentially
// good use case would be for a data member of a class that has many instances
// and the data member doesn't have many different values. Some examples would
// be Unicode block names or Unicode version names (see Ucd.h for examples).
//
// The current implementation uses u_int16_t so it can support up to ~65K unique
// symbols per type (an exception is thrown if this limit is exceeded). If there
// are more than 65K values then 'Symbol' was probably not a good design choice
// in the first place.
//
// Classes should derive from 'Symbol' and define 'Type', for example:
//   class TestSymbol : public Symbol<TestSymbol> {
//   public:
//     inline static const std::string Type{"TestSymbol"};
//     using Symbol::Symbol;
//   };

class BaseSymbol {
public:
  using Id = u_int16_t;

  // '0' is used as the 'id' for 'empty' symbols (non-empty start at '1')
  static constexpr auto Max{std::numeric_limits<Id>::max() - 1};

  [[nodiscard]] constexpr auto id() const { return _id; }

  [[nodiscard]] constexpr operator bool() const { return _id; }
protected:
  using Map = std::map<std::string, Id>;
  using List = std::vector<const std::string*>;

  constexpr explicit BaseSymbol(Id id) noexcept : _id{id} {}

  [[nodiscard]] static Id getId(
      const std::string& type, const std::string& name, Map&, List&);
private:
  const Id _id;
};

template<typename T> class Symbol : public BaseSymbol {
public:
  [[nodiscard]] static const std::string& type() { return T::Type; }

  // 'size' returns total unique symbols created (not including 'empty')
  [[nodiscard]] static auto size() { return _list.size(); }

  // 'exists' returns true if a symbol exists for the given (non-empty) name
  [[nodiscard]] static auto exists(const std::string& name) {
    return !name.empty() && _map.contains(name);
  }

  constexpr Symbol() noexcept : BaseSymbol{0} {}
  explicit Symbol(const std::string& name)
      : BaseSymbol{getId(type(), name, _map, _list)} {}

  [[nodiscard]] auto& name() const {
    return id() ? *_list.at(id() - 1) : EmptyString;
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
