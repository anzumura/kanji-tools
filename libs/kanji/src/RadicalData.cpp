#include <kt_kanji/KanjiData.h>
#include <kt_utils/ColumnFile.h>
#include <kt_utils/UnicodeBlock.h>

#include <algorithm>
#include <numeric>
#include <sstream>

namespace kanji_tools {

namespace {

using Count = std::map<KanjiTypes, int>;

void printCounts(KanjiDataRef data, const Count& c, bool summary = false) {
  const auto t{std::accumulate(c.begin(), c.end(), 0,
      [](const auto& x, const auto& y) { return x + y.second; })};
  data.out() << std::setfill(' ') << std::right << std::setw(4) << t << " (";
  for (const auto i : AllKanjiTypes) {
    if (const auto j{c.find(i)}; summary) {
      if (i != AllKanjiTypes[0]) data.out() << ' ';
      data.out() << (j == c.end() ? 0 : j->second);
    } else
      data.out() << std::setw(4) << (j == c.end() ? 0 : j->second);
    if (isNextNone(i)) break;
  }
  data.out() << (summary ? ")\n" : ") :");
}

} // namespace

RadicalRef RadicalData::find(Radical::Name name) const {
  checkLoaded();
  const auto i{_map.find(name)};
  if (i == _map.end()) throw DomainError{"name not found: " + name};
  return _radicals.at(i->second);
}

RadicalRef RadicalData::find(Radical::Number number) const {
  checkLoaded();
  if (!number || number > _radicals.size())
    throw DomainError(
        "'" + std::to_string(number) + "' is not a valid radical number");
  return _radicals.at(number - 1);
}

void RadicalData::load(const KanjiData::Path& file) {
  const ColumnFile::Column numberCol{"Number"}, nameCol{"Name"},
      longNameCol{"LongName"}, readingCol{"Reading"};
  for (ColumnFile f{file, {numberCol, nameCol, longNameCol, readingCol}};
       f.nextRow();) {
    const auto radicalNumber{f.getU8(numberCol)};
    if (radicalNumber != f.currentRow())
      f.error("radicals must be ordered by 'number'");
    std::stringstream radicals{f.get(nameCol)};
    Radical::AltForms altForms;
    String name, token;
    while (std::getline(radicals, token, ' '))
      if (name.empty())
        name = token;
      else
        altForms.emplace_back(token);
    _radicals.emplace_back(
        radicalNumber, name, altForms, f.get(longNameCol), f.get(readingCol));
    _map[name] = radicalNumber - 1;
  }
}

void RadicalData::print(KanjiDataRef data) const {
  data.log() << "Common Kanji Radicals (";
  for (auto i : AllKanjiTypes) {
    data.out() << i;
    if (isNextNone(i)) break;
    data.out() << ' ';
  }
  data.out() << "):\n";
  RadicalLists radicals;
  for (auto& i : data.nameMap())
    // only include 'Common Kanji' for now since a lot of the rare kanji don't
    // display properly - they just show up as '?' (𮧮)
    if (isCommonKanji(i.second->name()))
      radicals[i.second->radical()].emplace_back(i.second);
  printRadicalLists(data, radicals);
  printMissingRadicals(data, radicals);
}

void RadicalData::printRadicalLists(KanjiDataRef data, RadicalLists& radicals) {
  Count total;
  for (auto& i : radicals) {
    auto& l{i.second};
    std::sort(l.begin(), l.end(), KanjiData::OrderByStrokes);
    Count count;
    for (const auto& j : l) {
      ++count[j->type()];
      ++total[j->type()];
    }
    data.out() << i.first << ':';
    printCounts(data, count);
    size_t j{};
    for (; j < l.size() && j < MaxExamples; ++j)
      data.out() << ' ' << l[j]->name();
    if (j < l.size()) data.out() << " ...";
    data.out() << '\n';
  }
  data.log() << "  Total for " << radicals.size() << " radicals: ";
  printCounts(data, total, true);
}

void RadicalData::checkLoaded() const {
  if (_radicals.empty())
    throw DomainError("must call 'load' before calling 'find'");
}

void RadicalData::printMissingRadicals(
    KanjiDataRef data, const RadicalLists& radicals) const {
  std::vector<Radical> missingRadicals;
  for (auto& i : _radicals)
    if (radicals.find(i) == radicals.end()) missingRadicals.emplace_back(i);
  if (!missingRadicals.empty()) {
    data.log() << "  Found " << missingRadicals.size() << " radical"
               << (missingRadicals.size() > 1 ? "s" : "") << " with no Kanji:";
    for (const auto& i : missingRadicals) data.out() << ' ' << i;
    data.out() << '\n';
  }
}

} // namespace kanji_tools
