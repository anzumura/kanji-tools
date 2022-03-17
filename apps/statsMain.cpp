#include <kanji_tools/kanji/KanjiData.h>
#include <kanji_tools/stats/Stats.h>

using namespace kanji_tools;

int main(int argc, const char** argv) {
  try {
    const auto c{static_cast<u_int8_t>(argc)};
    Stats stats{c, argv, std::make_shared<KanjiData>(c, argv)};
  } catch (const std::exception& err) {
    std::cerr << err.what() << '\n';
    return 1;
  }
  return 0;
}
