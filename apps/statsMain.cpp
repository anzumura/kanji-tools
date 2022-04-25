#include <kanji_tools/kanji/KanjiData.h>
#include <kanji_tools/stats/Stats.h>

int main(int argc, const char** argv) {
  try {
    using kanji_tools::Args, kanji_tools::Stats, kanji_tools::KanjiData;
    const Args args{argc, argv};
    Stats stats{args, std::make_shared<KanjiData>(args)};
  } catch (const std::exception& err) {
    std::cerr << err.what() << '\n';
    return 1;
  }
  return 0;
}
