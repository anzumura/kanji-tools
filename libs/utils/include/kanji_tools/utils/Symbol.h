#pragma once

#include <map>
#include <string>
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
//     explicit TestSymbol(const std::string& name)
//         : Symbol<TestSymbol>{name} {}
//   };

class BaseSymbol {
public:
  using Id = u_int16_t;

  [[nodiscard]] Id id() const { return _id; }
protected:
  using Map = std::map<std::string, Id>;
  using List = std::vector<const std::string*>;

  explicit BaseSymbol(Id id) noexcept : _id{id} {}

  [[nodiscard]] static Id getId(
      const std::string& type, const std::string& name, Map&, List&);
private:
  const Id _id;
};

template<typename T> class Symbol : public BaseSymbol {
public:
  [[nodiscard]] static const std::string& type() { return T::Type; }
  [[nodiscard]] static auto size() { return _list.size(); }
  [[nodiscard]] static auto exists(const std::string& name) {
    return _map.contains(name);
  }

  [[nodiscard]] auto& name() const { return *_list.at(id()); }
protected:
  explicit Symbol(const std::string& name)
      : BaseSymbol{getId(type(), name, _map, _list)} {}
private:
  inline static Map _map;
  inline static List _list;
};

} // namespace kanji_tools
