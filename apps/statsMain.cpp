#include <kt_kanji/FileKanjiData.h>
#include <kt_stats/Stats.h>

int main(int argc, const char** argv) {
  using kanji_tools::Args, kanji_tools::Stats, kanji_tools::FileKanjiData;
  try {
    const Args args{argc, argv};
    Stats{args, std::make_shared<FileKanjiData>(args)};
  } catch (const std::exception& err) {
    std::cerr << err.what() << '\n';
    return 1;
  }
  return 0;
}
