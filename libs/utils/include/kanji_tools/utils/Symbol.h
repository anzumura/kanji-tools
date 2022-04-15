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

class BaseSymbol {
public:
  using Id = u_int16_t;

  [[nodiscard]] Id id() const { return _id; }
protected:
  BaseSymbol(Id id) noexcept : _id{id} {}

  [[nodiscard]] static Id check(size_t, const std::string&, const std::string&);
private:
  const Id _id;
};

template<typename T> class Symbol : public BaseSymbol {
public:
  [[nodiscard]] static const std::string& type() { return Type; }
  [[nodiscard]] static auto symbols() { return _list.size(); }

  [[nodiscard]] auto& name() const { return _list.at(id()); }
protected:
  Symbol(const std::string& symbol) : BaseSymbol{getId(symbol)} {}
private:
  using Map = std::map<std::string, Id>;
  using List = std::vector<std::string>;

  static const std::string Type; // must be defined for each derived class

  inline static Map _map;
  inline static List _list;

  [[nodiscard]] static Id getId(const std::string& symbol) {
    if (const auto i{_map.find(symbol)}; i != _map.end()) return i->second;
    const auto id{check(_list.size(), Type, symbol)};
    _map.emplace(symbol, id);
    _list.emplace_back(symbol);
    return id;
  }
};

} // namespace kanji_tools
