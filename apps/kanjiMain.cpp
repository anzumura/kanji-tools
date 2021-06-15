#include <kanji/Kanji.h>

#include <array>

using namespace kanji;

constexpr std::array<Grades, 8> grades = {Grades::G1, Grades::G2, Grades::G3, Grades::G4,
                                          Grades::G5, Grades::G6, Grades::S,  Grades::None};
constexpr std::array<Levels, 6> levels = {Levels::N5, Levels::N4, Levels::N3, Levels::N2, Levels::N1, Levels::None};

void noFreq(int f, bool brackets = false) {
  if (f) {
    if (brackets)
      std::cout << " (";
    else
      std::cout << ' ';
    std::cout << "No-Freq " << f;
    if (brackets) std::cout << ')';
  }
}

void countGrades(const KanjiLists::List& l) {
  int all = 0;
  for (auto i : grades) {
    const auto c = std::count_if(l.begin(), l.end(), [i](const auto& x) { return x->grade == i; });
    if (c) {
      all += c;
      std::cout << ">>> total for grade " << i << ": " << c;
      noFreq(std::count_if(l.begin(), l.end(), [i](const auto& x) { return x->grade == i && !x->frequency; }), true);
      std::cout << '\n';
    }
  }
  std::cout << ">>> total for all grades: " << all << '\n';
}

void countLevels(const KanjiLists::List& l1, const KanjiLists::List& l2, const KanjiLists::List& l3) {
  int all = 0;
  for (auto i : levels) {
    const auto c1 = std::count_if(l1.begin(), l1.end(), [i](const auto& x) { return x->level == i; });
    const auto c2 = std::count_if(l2.begin(), l2.end(), [i](const auto& x) { return x->level == i; });
    const auto c3 = std::count_if(l3.begin(), l3.end(), [i](const auto& x) { return x->level == i; });
    const auto c = c1 + c2 + c3;
    if (c) {
      all += c;
      std::cout << ">>> total for level " << i << ": " << c << " (";
      if (c1) {
        std::cout << "Jouyou " << c1;
        noFreq(std::count_if(l1.begin(), l1.end(), [i](const auto& x) { return x->level == i && !x->frequency; }));
        if (c2 || c3) std::cout << ", ";
      }
      if (c2) {
        std::cout << "Jinmei " << c2;
        noFreq(std::count_if(l2.begin(), l2.end(), [i](const auto& x) { return x->level == i && !x->frequency; }));
        if (c3) std::cout << ", ";
      }
      if (c3) {
        std::cout << "Other " << c3;
        noFreq(std::count_if(l3.begin(), l3.end(), [i](const auto& x) { return x->level == i && !x->frequency; }));
      }
      std::cout << ")\n";
    }
  }
  std::cout << ">>> total for all levels: " << all << '\n';
}

auto noFrequency(const KanjiLists::List& l) {
  return std::count_if(l.begin(), l.end(), [](const auto& x){ return !x->frequency; });
}

int main(int argc, char** argv) {
  KanjiLists l(argc, argv);
  const auto& jouyou = l.jouyouKanji();
  const auto& jinmei = l.jinmeiKanji();
  const auto& other = l.otherKanji();
  auto total = jouyou.size() + jinmei.size() + other.size();
  std::cout << ">>> loaded " << total << " Kanji (Jouyou: " << jouyou.size() << " Jinmei: " << jinmei.size()
            << " Other: " << other.size() << ")\n";
  //std::cout << ">>> No-Freq"
  countGrades(jouyou);
  countLevels(jouyou, jinmei, other);
  // for (const auto& i : kanji)
  //   std::cout << i->name << ": " << i->grade << ", " << i->level << ", " << i->frequency << '\n';
  return 0;
}
