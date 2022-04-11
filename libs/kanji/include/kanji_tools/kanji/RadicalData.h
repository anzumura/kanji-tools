#pragma once

#include <kanji_tools/kanji/Radical.h>

#include <filesystem>
#include <map>
#include <memory>

namespace kanji_tools {

// 'RadicalData': holds data loaded from 'radicals.txt' (214 official Radicals).
class RadicalData {
public:
  using Map = std::map<std::string, Radical::Number>;
  using List = std::vector<Radical>;

  RadicalData() {}

  RadicalData(const RadicalData&) = delete;
  RadicalData& operator=(const RadicalData&) = delete;

  // 'find' by the ideograph code in utf8 (not the unicode radical code). For
  // example, Radical number 30 (口) is Unicode 53E3, but has another 'Unicode
  // Radical' value of 2F1D
  [[nodiscard]] RadicalRef find(const std::string&) const;

  // 'find' by the official Radical Number (one greater than index in _radicals)
  [[nodiscard]] RadicalRef find(Radical::Number) const;

  // 'load' and 'print' are called by 'KanjiData'
  void load(const std::filesystem::path&);
  void print(const class Data&) const;
private:
  // 'MaxExamples' controls how many examples are printed for each radical by
  // the above 'print' function (examples are sorted by assending stroke count).
  enum Values { MaxExamples = 12 };

  void checkLoaded() const;

  using KanjiList = std::vector<std::shared_ptr<class Kanji>>;
  using RadicalLists = std::map<Radical, KanjiList>;

  void printRadicalLists(const Data&, RadicalLists&) const;
  void printMissingRadicals(const Data&, const RadicalLists&) const;

  // '_radicals' is populated from radicals.txt and the index in the vector is
  // one less than the actual Radical.number().
  List _radicals;

  // '_map' maps from the Radical name (ideograph) to the index in _radicals.
  Map _map;
};

} // namespace kanji_tools
