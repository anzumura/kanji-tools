#include <kanji/KanaConvert.h>

#include <iostream>

using namespace kanji;

int main(int argc, const char** argv) {
  try {
    KanaConvert converter;
  } catch(const std::exception& err) {
    std::cerr << err.what() << '\n';
    return 1;
  }
  return 0;
}
