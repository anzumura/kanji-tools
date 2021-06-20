#include <kanji/Data.h>
#include <kanji/Kanji.h>

#include <numeric>

using namespace kanji;

constexpr std::array<Grades, 8> grades = {Grades::G1, Grades::G2, Grades::G3, Grades::G4,
                                          Grades::G5, Grades::G6, Grades::S,  Grades::None};
constexpr std::array<Levels, 6> levels = {Levels::N5, Levels::N4, Levels::N3, Levels::N2, Levels::N1, Levels::None};
constexpr std::array<Types, 7> types = {Types::Jouyou, Types::Jinmei, Types::LinkedJinmei, Types::LinkedOld,
                                        Types::Other,  Types::Extra,  Types::None};

void noFreq(int f, bool brackets = false) {
  if (f) {
    if (brackets)
      std::cout << " (";
    else
      std::cout << ' ';
    std::cout << "nf " << f;
    if (brackets) std::cout << ')';
  }
}

void countGrades(const Data::List& l) {
  std::cout << ">>> Grade breakdown:\n";
  int all = 0;
  for (auto i : grades) {
    auto grade = [i](const auto& x) { return x->grade() == i; };
    auto gradeCount = std::count_if(l.begin(), l.end(), grade);
    if (gradeCount) {
      all += gradeCount;
      std::cout << ">>>   Total for grade " << i << ": " << gradeCount;
      noFreq(std::count_if(l.begin(), l.end(), [&grade](const auto& x) { return grade(x) && !x->frequency(); }), true);
      std::cout << " (";
      for (auto level : levels) {
        const auto gradeLevelCount =
          std::count_if(l.begin(), l.end(), [&grade, level](const auto& x) { return grade(x) && x->level() == level; });
        if (gradeLevelCount) {
          gradeCount -= gradeLevelCount;
          std::cout << level << ' ' << gradeLevelCount;
          if (gradeCount) std::cout << ", ";
        }
      }
      std::cout << ")\n";
    }
  }
  std::cout << ">>>   Total for all grades: " << all << '\n';
}

void countLevels(const std::vector<const Data::List*>& lists) {
  std::cout << ">>> Level breakdown:\n";
  int total = 0;
  for (auto level : levels) {
    std::vector<int> counts;
    for (const auto& l : lists)
      counts.push_back(std::count_if(l->begin(), l->end(), [level](const auto& x) { return x->level() == level; }));
    int levelTotal = std::reduce(counts.begin(), counts.end());
    if (levelTotal) {
      total += levelTotal;
      std::cout << ">>>   Total for level " << level << ": " << levelTotal << " (";
      for (int j = 0; j < counts.size(); ++j)
        if (counts[j]) {
          levelTotal -= counts[j];
          std::cout << types[j] << ' ' << counts[j];
          noFreq(std::count_if(lists[j]->begin(), lists[j]->end(),
                               [level](const auto& x) { return x->level() == level && !x->frequency(); }));
          if (levelTotal) std::cout << ", ";
        }
      std::cout << ")\n";
    }
  }
  std::cout << ">>>   Total for all levels: " << total << '\n';
}

void countRadicals(const std::vector<const Data::List*>& lists, const Data::RadicalMap& allRadicals) {
  std::cout << ">>> Radical breakdown - total count for each radical is followed by (Jouyou Jinmei Extra) counts:\n";
  std::map<Radical, Data::List> radicals;
  for (const auto& i : lists) {
    Data::List sorted(*i);
    std::sort(sorted.begin(), sorted.end(), [](const auto& x, const auto& y) { return x->strokes() - y->strokes(); });
    for (const auto& j : sorted)
      radicals[static_cast<const FileListKanji&>(*j).radical()].push_back(j);
  }
  int jouyou = 0, jinmei = 0, extra = 0;
  for (const auto& i : radicals) {
    int jo = 0, ji = 0, ex = 0;
    for (const auto& j : i.second)
      switch (j->type()) {
      case Types::Jouyou: ++jo; break;
      case Types::Jinmei: ++ji; break;
      default: ++ex; break;
      }
    auto counts = std::to_string(jo) + ' ' + std::to_string(ji) + ' ' + std::to_string(ex) + ')';
    std::cout << ">>> " << i.first << ':' << std::setfill(' ') << std::right << std::setw(4) << i.second.size() << " ("
              << std::left << std::setw(9) << counts << ':';
    jouyou += jo;
    jinmei += ji;
    extra += ex;
    Types oldType = i.second[0]->type();
    for (const auto& j : i.second) {
      if (j->type() != oldType) {
        std::cout << "、";
        oldType = j->type();
      }
      std::cout << ' ' << *j;
    }
    std::cout << '\n';
  }
  std::cout << ">>>   Total for " << radicals.size() << " radicals: " << jouyou + jinmei + extra << " (Jouyou "
            << jouyou << " Jinmei " << jinmei << " Extra " << extra << ")\n";
  std::vector<Radical> missingRadicals;
  for (const auto& i : allRadicals)
    if (radicals.find(i.second) == radicals.end()) missingRadicals.push_back(i.second);
  if (!missingRadicals.empty()) {
    std::cout << ">>>   Found " << missingRadicals.size() << " radicals with no kanji:";
    for (const auto& i : missingRadicals)
      std::cout << ' ' << i;
    std::cout << '\n';
  }
}

template<typename T> void count(const std::vector<const Data::List*>& lists, const std::string& name, T pred) {
  std::vector<int> counts;
  for (const auto& l : lists)
    counts.push_back(std::count_if(l->begin(), l->end(), pred));
  const auto count = std::reduce(counts.begin(), counts.end());
  if (count) {
    std::cout << ">>> " << name << ' ' << count << " (";
    for (int i = 0; i < counts.size(); ++i) {
      if (i) std::cout << ", ";
      std::cout << types[i] << ' ' << counts[i];
    }
    std::cout << ")\n";
  }
}

int main(int argc, char** argv) {
  Data data(argc, argv);
  std::vector<const Data::List*> lists;
  lists.push_back(&data.jouyouKanji());
  lists.push_back(&data.jinmeiKanji());
  lists.push_back(&data.linkedJinmeiKanji());
  lists.push_back(&data.linkedOldKanji());
  lists.push_back(&data.otherKanji());
  lists.push_back(&data.extraKanji());
  decltype(lists) mayHaveOld(lists.begin(), lists.begin() + 2);
  // linked and other lists don't have radicals right now
  decltype(lists) hasRadical = {lists[0], lists[1], lists[lists.size() - 1]};
  int total = 0;
  for (const auto& l : lists)
    total += l->size();
  std::cout << ">>> Loaded " << total << " Kanji (";
  for (int i = 0; i < lists.size(); ++i) {
    if (i) std::cout << ' ';
    std::cout << types[i] << ' ' << lists[i]->size();
  }
  std::cout << ")\n";
  count(lists, "NF (no-frequency)", [](const auto& x) { return !x->frequency(); });
  count(lists, "Has Strokes", [](const auto& x) { return x->strokes() != 0; });
  count(mayHaveOld, "Old Forms", [](const auto& x) { return x->oldName().has_value(); });
  // some old kanjis should have a non-zero frequency
  count(mayHaveOld, "  Old Has Frequency", [&data](const auto& x) { return x->oldFrequency(data) != 0; });
  // some old kanjis have stroke counts
  count(mayHaveOld, "  Old Has Strokes", [&data](const auto& x) { return x->oldStrokes(data) != 0; });
  // no old kanjis should have a non-None level
  count(mayHaveOld, "  Old Has Level", [&data](const auto& x) { return x->oldLevel(data) != Levels::None; });
  // old kanjis should only have types of LinkedJinmei, Other or None
  for (auto i : types)
    count(mayHaveOld, std::string("  Old is type ") + toString(i),
          [&data, i](const auto& x) { return x->oldName().has_value() && x->oldType(data) == i; });
  countGrades(*lists[0]); // only Jouyou list has Grades
  countLevels(lists);
  countRadicals(hasRadical, data.radicals());
  // const auto& k = (*lists[0])[0];
  // std::cout << "First Jouyou: " << k->name() << " - len: " << length(k->name()) << '\n';
  // for (const auto& i : *lists[0])
  //   std::cout << i->name() << ": " << i->grade() << ", " << i->level() << ", " << i->frequency() << '\n';
  return 0;
}
