#include <kanji/Kanji.h>

int main(int argc, char** argv) {
  kanji::KanjiLists l(argc, argv);
  const auto& kanji = l.jouyou();
  std::cout << "loaded " << kanji.size() << " jouyou kanji\n";
  //for (const auto& i : kanji)
  //  std::cout << i->name << ": " << i->grade << ", " << i->level << ", " << i->frequency << '\n';
  return 0;
}
