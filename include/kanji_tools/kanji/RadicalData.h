#ifndef KANJI_TOOLS_KANJI_RADICAL_DATA_H
#define KANJI_TOOLS_KANJI_RADICAL_DATA_H

#include <kanji_tools/kanji/Radical.h>

#include <filesystem>
#include <iostream>
#include <map>

namespace kanji_tools {

class Data;

// 'RadicalData': holds data loaded from 'radicals.txt' (the 214 official Kanji Radicals).
class RadicalData {
public:
  using Map = std::map<std::string, int>;
  using List = std::vector<Radical>;

  RadicalData() {}
  RadicalData(const RadicalData&) = delete;

  // 'find' by the ideograph code in utf8 (not the unicode radical code). For example,
  // Radical number 30 (口) is Unicode 53E3, but has another 'Unicode Radical' value of 2F1D
  const Radical& find(const std::string& name) const {
    auto i = _map.find(name);
    if (i == _map.end()) throw std::domain_error("name not found: " + name);
    return _radicals.at(i->second);
  }

  // 'find' by the official Radical Number (which is one greater than index in _radicals)
  const Radical& find(int number) const { return _radicals.at(number - 1); }

  // 'load' and 'print' are called by 'KanjiData'
  void load(const std::filesystem::path&);
  void print(const Data&) const;
private:
  // 'MaxExamples' controls how many examples are printed for each radical by the above 'print'
  // function (examples are sorted by assending stroke count).
  enum Values { MaxExamples = 12 };
  // '_radicals' is populated from radicals.txt and the index in the vector is one less than
  // the actual Radical.number().
  List _radicals;

  // '_map' maps from the Radical name (ideograph) to the index in _radicals.
  Map _map;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_KANJI_RADICAL_DATA_H
