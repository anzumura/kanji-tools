#include <kt_kana/KanaConvert.h>

int main(int argc, const char** argv) {
  try {
    kanji_tools::KanaConvert{{argc, argv}};
  } catch (const std::exception& err) {
    std::cerr << err.what() << '\n';
    return 1;
  }
  return 0;
}
