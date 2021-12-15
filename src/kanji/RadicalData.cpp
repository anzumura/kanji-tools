#include <kanji_tools/kanji/Data.h>

#include <fstream>
#include <numeric>
#include <sstream>

namespace kanji_tools {

void RadicalData::load(const std::filesystem::path& file) {
  int lineNum = 1, numberCol = -1, nameCol = -1, longNameCol = -1, readingCol = -1;
  auto error = [&lineNum, &file](const std::string& s, bool printLine = true) {
    Data::usage(s + (printLine ? " - line: " + std::to_string(lineNum) : Ucd::EmptyString) +
                ", file: " + file.string());
  };
  auto setCol = [&file, &error](int& col, int pos) {
    if (col != -1) error("column " + std::to_string(pos) + " has duplicate name");
    col = pos;
  };
  std::ifstream f(file);
  std::array<std::string, 4> cols;
  for (std::string line; std::getline(f, line); ++lineNum) {
    int pos = 0;
    if (std::stringstream ss(line); numberCol == -1) {
      for (std::string token; std::getline(ss, token, '\t'); ++pos)
        if (token == "Number")
          setCol(numberCol, pos);
        else if (token == "Name")
          setCol(nameCol, pos);
        else if (token == "LongName")
          setCol(longNameCol, pos);
        else if (token == "Reading")
          setCol(readingCol, pos);
        else
          error("unrecognized column '" + token + "'", false);
      if (pos != cols.size()) error("not enough columns", false);
    } else {
      for (std::string token; std::getline(ss, token, '\t'); ++pos) {
        if (pos == cols.size()) error("too many columns");
        cols[pos] = token;
      }
      if (pos != cols.size())
        error("not enough columns - got " + std::to_string(pos) + ", wanted " + std::to_string(cols.size()));
      const int radicalNumber = Data::toInt(cols[numberCol]);
      if (radicalNumber + 1 != lineNum) error("radicals must be ordered by 'number'");
      std::stringstream radicals(cols[nameCol]);
      Radical::AltForms altForms;
      std::string name, token;
      while (std::getline(radicals, token, ' '))
        if (name.empty())
          name = token;
        else
          altForms.emplace_back(token);
      _radicals.emplace_back(radicalNumber, name, altForms, cols[longNameCol], cols[readingCol]);
    }
  }
  for (auto& i : _radicals)
    _map[i.name()] = i.number() - 1;
}

void RadicalData::print(const Data& data) const {
  data.log() << "Radical breakdown - Total (";
  for (auto i : AllKanjiTypes) {
    data.out() << i;
    if (i == secondLast(AllKanjiTypes)) break;
    data.out() << ' ';
  }
  data.out() << "):\n";
  std::map<Radical, Data::List> radicals;
  for (auto& i : data.kanjiNameMap())
    radicals[i.second->radical()].push_back(i.second);
  using Count = std::map<KanjiTypes, int>;
  Count total;
  auto printCounts = [&data](const Count& c, bool summary = false) {
    const int t = std::accumulate(c.begin(), c.end(), 0, [](const auto& x, const auto& y) { return x + y.second; });
    data.out() << std::setfill(' ') << std::right << std::setw(4) << t << " (";
    for (auto i : AllKanjiTypes) {
      auto j = c.find(i);
      if (summary) {
        if (i != AllKanjiTypes[0]) data.out() << ' ';
        data.out() << (j == c.end() ? 0 : j->second);
      } else
        data.out() << std::setw(4) << (j == c.end() ? 0 : j->second);
      if (i == secondLast(AllKanjiTypes)) break;
    }
    data.out() << (summary ? ")\n" : ") :");
  };
  for (auto& i : radicals) {
    Data::List& l = i.second;
    std::sort(l.begin(), l.end(), [](const auto& x, const auto& y) { return x->strokes() < y->strokes(); });
    Count count;
    for (const auto& j : l) {
      ++count[j->type()];
      ++total[j->type()];
    }
    data.out() << i.first << ':';
    printCounts(count);
    int j = 0;
    for (; j < l.size() && j < MaxExamples; ++j)
      data.out() << ' ' << l[j]->name();
    if (j < l.size()) data.out() << " ...";
    data.out() << '\n';
  }
  data.log() << "  Total for " << radicals.size() << " radicals: ";
  printCounts(total, true);
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

} // namespace kanji_tools