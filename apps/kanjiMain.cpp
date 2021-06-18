#include <kanji/Kanji.h>

#include <numeric>

using namespace kanji;

constexpr std::array<Grades, 8> grades = {Grades::G1, Grades::G2, Grades::G3, Grades::G4,
                                          Grades::G5, Grades::G6, Grades::S,  Grades::None};
constexpr std::array<Levels, 6> levels = {Levels::N5, Levels::N4, Levels::N3, Levels::N2, Levels::N1, Levels::None};
constexpr std::array<Types, 5> types = {Types::Jouyou, Types::Jinmei, Types::Extra, Types::Other, Types::None};

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

void countGrades(const KanjiLists::List& l) {
  std::cout << "<<< Grade breakdown:\n";
  int all = 0;
  for (auto i : grades) {
    const auto c = std::count_if(l.begin(), l.end(), [i](const auto& x) { return x->grade() == i; });
    if (c) {
      all += c;
      std::cout << ">>>   Total for grade " << i << ": " << c;
      noFreq(std::count_if(l.begin(), l.end(), [i](const auto& x) { return x->grade() == i && !x->frequency(); }),
             true);
      std::cout << '\n';
    }
  }
  std::cout << ">>>   Total for all grades: " << all << '\n';
}

void countLevels(const std::vector<const KanjiLists::List*>& lists) {
  std::cout << "<<< Level breakdown:\n";
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

template<typename T> void count(const std::vector<const KanjiLists::List*>& lists, const std::string& name, T pred) {
  std::vector<int> counts;
  for (const auto& l : lists)
    counts.push_back(std::count_if(l->begin(), l->end(), pred));
  std::cout << ">>> " << name << ' ' << std::reduce(counts.begin(), counts.end()) << " (";
  for (int i = 0; i < counts.size(); ++i) {
    if (i) std::cout << ' ';
    std::cout << types[i] << ' ' << counts[i];
  }
  std::cout << ")\n";
}

int main(int argc, char** argv) {
  KanjiLists l(argc, argv);
  std::vector<const KanjiLists::List*> lists;
  lists.push_back(&l.jouyouKanji());
  lists.push_back(&l.jinmeiKanji());
  lists.push_back(&l.extraKanji());
  lists.push_back(&l.otherKanji());
  decltype(lists) fileLists(lists.begin(), lists.end() - 1);
  decltype(lists) officialLists(lists.begin(), lists.end() - 2);
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
  count(lists, "Old Forms", [](const auto& x) { return x->oldName().has_value(); });
  count(fileLists, "Has Strokes", [](const auto& x) { return static_cast<const FileListKanji&>(*x).strokes() != 0; });
  // some old kanjis should have a non-zero frequency
  count(officialLists, "Old Has Frequency", [&l](const KanjiLists::Entry& x) { return x->oldFrequency(l) != 0; });
  // some old kanjis have stroke counts
  count(officialLists, "Old Has Strokes", [&l](const KanjiLists::Entry& x) { return x->oldStrokes(l) != 0; });
  // no old kanjis should have a non-None level
  count(officialLists, "Old Has Level", [&l](const KanjiLists::Entry& x) { return x->oldLevel(l) != Levels::None; });
  // check if old kanjis all have
  for (auto i : types)
    count(officialLists, std::string("Old is type ") + toString(i),
          [&l, i](const KanjiLists::Entry& x) { return x->oldType(l) == i; });
  countGrades(*lists[0]); // only Jouyou list has Grades
  countLevels(lists);
  // for (const auto& i : *lists[0])
  //   std::cout << i->name() << ": " << i->grade() << ", " << i->level() << ", " << i->frequency() << '\n';
  return 0;
}
