#include <kanji_tools/kanji/KanjiData.h>
#include <kanji_tools/stats/Stats.h>

using namespace kanji_tools;

int main(int argc, const char** argv) {
  try {
    Stats stats(argc, argv, std::make_shared<KanjiData>(argc, argv));
  } catch (const std::exception& err) {
    std::cerr << err.what() << '\n';
    return 1;
  }
  return 0;
}
