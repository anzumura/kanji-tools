#include <kanji/KanjiQuiz.h>

using namespace kanji;

int main(int argc, const char** argv) {
  try {
    KanjiQuiz data(argc, argv);
    data.quiz();
  } catch(const std::exception& err) {
    std::cerr << err.what() << '\n';
    return 1;
  }
  return 0;
}
