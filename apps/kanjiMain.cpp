#include <kanji/Kanji.h>

#include <array>

using namespace kanji;

constexpr std::array<Grades, 8> grades = {Grades::G1, Grades::G2, Grades::G3, Grades::G4,
                                          Grades::G5, Grades::G6, Grades::S,  Grades::None};
constexpr std::array<Levels, 6> levels = {Levels::N5, Levels::N4, Levels::N3, Levels::N2, Levels::N1, Levels::None};

void countGrades(const KanjiLists::List& jouyou) {
  int all = 0;
  for (auto i : grades) {
    auto c = std::count_if(jouyou.begin(), jouyou.end(), [i](const auto& x) { return x->grade == i; });
    if (c) {
      std::cout << ">>> total for grade " << i << ": " << c << '\n';
      all += c;
    }
  }
  std::cout << ">>> total for all grades: " << all << '\n';
}

void countLevels(const KanjiLists::List& l, const std::string& name) {
  int all = 0;
  for (auto i : levels) {
    auto c = std::count_if(l.begin(), l.end(), [i](const auto& x) { return x->level == i; });
    if (c) {
      std::cout << ">>> " << name << " total for level " << i << ": " << c << '\n';
      all += c;
    }
  }
  std::cout << ">>> " << name << " total for all levels: " << all << '\n';
}

int main(int argc, char** argv) {
  KanjiLists l(argc, argv);
  const auto& jouyou = l.jouyou();
  const auto& nonJouyou = l.nonJouyou();
  std::cout << ">>> loaded " << jouyou.size() << " Jouyou Kanji\n";
  std::cout << ">>> loaded " << nonJouyou.size() << " Non-Jouyou Kanji\n";
  countGrades(jouyou);
  countLevels(jouyou, "Jouyou");
  countLevels(nonJouyou, "Non-Jouyou");
  // for (const auto& i : kanji)
  //  std::cout << i->name << ": " << i->grade << ", " << i->level << ", " << i->frequency << '\n';
  return 0;
}
