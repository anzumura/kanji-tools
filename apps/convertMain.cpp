#include <kanji_tools/kana/KanaConvert.h>

using namespace kanji_tools;

int main(int argc, const char** argv) {
  try {
    KanaConvert{{argc, argv}};
  } catch (const std::exception& err) {
    std::cerr << err.what() << '\n';
    return 1;
  }
  return 0;
}
