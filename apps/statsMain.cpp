#include <kanji_tools/kanji/RealKanjiData.h>
#include <kanji_tools/stats/Stats.h>

int main(int argc, const char** argv) {
  using kanji_tools::Args, kanji_tools::Stats, kanji_tools::RealKanjiData;
  try {
    const Args args{argc, argv};
    Stats{args, std::make_shared<RealKanjiData>(args)};
  } catch (const std::exception& err) {
    std::cerr << err.what() << '\n';
    return 1;
  }
  return 0;
}
