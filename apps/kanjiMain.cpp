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

void countLevels(const KanjiLists::List& l1, const KanjiLists::List& l2) {
  int all = 0;
  for (auto i : levels) {
    const auto c1 = std::count_if(l1.begin(), l1.end(), [i](const auto& x) { return x->level == i; });
    const auto c2 = std::count_if(l2.begin(), l2.end(), [i](const auto& x) { return x->level == i; });
    const auto c = c1 + c2;
    if (c) {
      all += c;
      std::cout << ">>> total for level " << i << ": " << c << " (";
      if (c1) {
        std::cout << "Jouyou " << c1;
        noFreq(std::count_if(l1.begin(), l1.end(), [i](const auto& x) { return x->level == i && !x->frequency; }));
        if (c2) std::cout << ", ";
      }
      if (c2) {
        std::cout << "Non-Jouyou " << c2;
        noFreq(std::count_if(l2.begin(), l2.end(), [i](const auto& x) { return x->level == i && !x->frequency; }));
      }
      std::cout << ")\n";
    }
  }
  std::cout << ">>> total for all levels: " << all << '\n';
}

int main(int argc, char** argv) {
  KanjiLists l(argc, argv);
  const auto& jouyou = l.jouyou();
  const auto& nonJouyou = l.nonJouyou();
  std::cout << ">>> loaded " << jouyou.size() << " Jouyou Kanji\n";
  std::cout << ">>> loaded " << nonJouyou.size() << " Non-Jouyou Kanji\n";
  countGrades(jouyou);
  countLevels(jouyou, nonJouyou);
  // for (const auto& i : kanji)
  //  std::cout << i->name << ": " << i->grade << ", " << i->level << ", " << i->frequency << '\n';
  return 0;
}
