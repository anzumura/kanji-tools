#include <kanji/FileStats.h>
#include <kanji/KanjiData.h>

using namespace kanji;

int main(int argc, const char** argv) {
  try {
    FileStats data(argc, argv, std::make_shared<KanjiData>(argc, argv));
  } catch(const std::exception& err) {
    std::cerr << err.what() << '\n';
    return 1;
  }
  return 0;
}
