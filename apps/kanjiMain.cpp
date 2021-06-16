#include <kanji/Kanji.h>

#include <array>
#include <numeric>

using namespace kanji;

constexpr std::array<Grades, 8> grades = {Grades::G1, Grades::G2, Grades::G3, Grades::G4,
                                          Grades::G5, Grades::G6, Grades::S,  Grades::None};
constexpr std::array<Levels, 6> levels = {Levels::N5, Levels::N4, Levels::N3, Levels::N2, Levels::N1, Levels::None};
constexpr std::array<Types, 4> types = {Types::Jouyou, Types::Jinmei, Types::Extra, Types::Other};

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
  int all = 0;
  for (auto i : grades) {
    const auto c = std::count_if(l.begin(), l.end(), [i](const auto& x) { return x->grade == i; });
    if (c) {
      all += c;
      std::cout << ">>>   Total for grade " << i << ": " << c;
      noFreq(std::count_if(l.begin(), l.end(), [i](const auto& x) { return x->grade == i && !x->frequency; }), true);
      std::cout << '\n';
    }
  }
  std::cout << ">>> Total for all grades: " << all << '\n';
}

void countLevels(const std::vector<const KanjiLists::List*>& lists) {
  int total = 0;
  for (auto level : levels) {
    std::vector<int> counts;
    for (const auto& l : lists)
      counts.push_back(std::count_if(l->begin(), l->end(), [level](const auto& x) { return x->level == level; }));
    int levelTotal = std::reduce(counts.begin(), counts.end());
    if (levelTotal) {
      total += levelTotal;
      std::cout << ">>>   Total for level " << level << ": " << levelTotal << " (";
      for (int j = 0; j < counts.size(); ++j)
        if (counts[j]) {
          total -= counts[j];
          std::cout << types[j] << ' ' << counts[j];
          noFreq(std::count_if(lists[j]->begin(), lists[j]->end(),
                               [level](const auto& x) { return x->level == level && !x->frequency; }));
          if (total) std::cout << ", ";
        }
      std::cout << ")\n";
    }
  }
  std::cout << ">>> Total for all levels: " << total << '\n';
}

auto noFrequency(const KanjiLists::List& l) {
  return std::count_if(l.begin(), l.end(), [](const auto& x) { return !x->frequency; });
}

int main(int argc, char** argv) {
  KanjiLists l(argc, argv);
  const auto& jouyou = l.jouyouKanji();
  const auto& jinmei = l.jinmeiKanji();
  const auto& extra = l.extraKanji();
  const auto& other = l.otherKanji();
  auto total = jouyou.size() + jinmei.size() + extra.size() + other.size();
  std::cout << ">>> Loaded " << total << " Kanji (Jouyou: " << jouyou.size() << " Jinmei: " << jinmei.size()
            << " Extra: " << extra.size() << " Other: " << other.size() << ")\n";
  auto jouyouNF = noFrequency(jouyou);
  auto jinmeiNF = noFrequency(jinmei);
  auto extraNF = noFrequency(extra);
  auto otherNF = noFrequency(other);
  std::cout << ">>> NF (no-frequency) " << jouyouNF + jinmeiNF + extraNF + otherNF << " (Jouyou: " << jouyouNF
            << " Jinmei: " << jinmeiNF << " Extra: " << extraNF << " Other: " << otherNF << ")\n";
  countGrades(jouyou);
  countLevels({&jouyou, &jinmei, &extra, &other});
  // for (const auto& i : kanji)
  //   std::cout << i->name << ": " << i->grade << ", " << i->level << ", " << i->frequency << '\n';
  return 0;
}
