#include <kanji/KanjiCount.h>

using namespace kanji;

int main(int argc, const char** argv) {
  try {
    KanjiCount data(argc, argv);
  } catch(const std::exception& err) {
    std::cerr << err.what() << '\n';
    return 1;
  }
  return 0;
}
