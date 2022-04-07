#include <kanji_tools/utils/BlockRange.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>

using namespace kanji_tools;
namespace fs = std::filesystem;

int main(int argc, const char** argv) {
  if (argc < 2) {
    std::cerr << "specify a file to format\n";
    return 1;
  }
  const auto file{argv[1]};
  const std::wregex endsWithKanji{std::wstring{L"["} + KanjiRange() + L"]{1}$"},
      allKana{std::wstring{L"^["} + KanaRange() + L"]+$"};
  std::fstream f{file};
  std::string line, prevLine;
  auto prevLineEndedWithKanji{false};
  while (std::getline(f, line)) {
    if (auto wline{fromUtf8ToWstring(line)}; prevLineEndedWithKanji) {
      if (std::regex_search(wline, allKana)) {
        prevLineEndedWithKanji = false;
        // The previous line ended with kanji and current line is all hiragana
        // so assume the current line is furigana for the kanji at the end of
        // the previous line and print it inside wide brackets.
        std::cout << prevLine << "（" << line << "）";
      } else {
        std::cout << prevLine << '\n';
        if (!std::regex_search(wline, endsWithKanji)) {
          prevLineEndedWithKanji = false;
          std::cout << line << '\n';
        }
      }
    } else if (std::regex_search(wline, endsWithKanji))
      prevLineEndedWithKanji = true;
    else
      std::cout << line << '\n';
    prevLine = line;
  }
  return 0;
}
