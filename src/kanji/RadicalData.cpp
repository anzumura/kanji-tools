#include <kanji_tools/kanji/Data.h>
#include <kanji_tools/utils/ColumnFile.h>

#include <numeric>
#include <sstream>

namespace kanji_tools {

void RadicalData::load(const std::filesystem::path& file) {
  ColumnFile::Column number("Number"), name("Name"), longName("LongName"), reading("Reading");
  for (ColumnFile f(file, {number, name, longName, reading}); f.nextRow();) {
    const int radicalNumber = Data::toInt(f.get(number));
    if (radicalNumber != f.currentRow()) f.error("radicals must be ordered by 'number'");
    std::stringstream radicals(f.get(name));
    Radical::AltForms altForms;
    std::string name, token;
    while (std::getline(radicals, token, ' '))
      if (name.empty())
        name = token;
      else
        altForms.emplace_back(token);
    _radicals.emplace_back(radicalNumber, name, altForms, f.get(longName), f.get(reading));
    _map[name] = radicalNumber - 1;
  }
}

void RadicalData::print(const Data& data) const {
  data.log() << "Radical breakdown - Total (";
  for (auto i : AllKanjiTypes) {
    data.out() << i;
    if (i == secondLast(AllKanjiTypes)) break;
    data.out() << ' ';
  }
  data.out() << "):\n";
  RadicalLists radicals;
  for (auto& i : data.kanjiNameMap())
    radicals[i.second->radical()].push_back(i.second);
  printRadicalLists(data, radicals);
  printMissingRadicals(data, radicals);
}

void RadicalData::printRadicalLists(const Data& data, RadicalLists& radicals) const {
  Count total;
  for (auto& i : radicals) {
    KanjiList& l = i.second;
    std::sort(l.begin(), l.end(), [](const auto& x, const auto& y) { return x->strokes() < y->strokes(); });
    Count count;
    for (const auto& j : l) {
      ++count[j->type()];
      ++total[j->type()];
    }
    data.out() << i.first << ':';
    printCounts(data, count);
    int j = 0;
    for (; j < l.size() && j < MaxExamples; ++j)
      data.out() << ' ' << l[j]->name();
    if (j < l.size()) data.out() << " ...";
    data.out() << '\n';
  }
  data.log() << "  Total for " << radicals.size() << " radicals: ";
  printCounts(data, total, true);
}

void RadicalData::printMissingRadicals(const Data& data, const RadicalLists& radicals) const {
  std::vector<Radical> missingRadicals;
  for (const auto& i : _radicals)
    if (radicals.find(i) == radicals.end()) missingRadicals.push_back(i);
  if (!missingRadicals.empty()) {
    data.log() << "  Found " << missingRadicals.size() << " radicals with no kanji:";
    for (const auto& i : missingRadicals)
      data.out() << ' ' << i;
    data.out() << '\n';
  }
}

void RadicalData::printCounts(const Data& data, const Count& c, bool summary) const {
  const int t = std::accumulate(c.begin(), c.end(), 0, [](const auto& x, const auto& y) { return x + y.second; });
  data.out() << std::setfill(' ') << std::right << std::setw(4) << t << " (";
  for (auto i : AllKanjiTypes) {
    if (auto j = c.find(i); summary) {
      if (i != AllKanjiTypes[0]) data.out() << ' ';
      data.out() << (j == c.end() ? 0 : j->second);
    } else
      data.out() << std::setw(4) << (j == c.end() ? 0 : j->second);
    if (i == secondLast(AllKanjiTypes)) break;
  }
  data.out() << (summary ? ")\n" : ") :");
}

} // namespace kanji_tools
