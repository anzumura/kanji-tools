#include <kanji_tools/utils/Args.h>
#include <kanji_tools/utils/BlockRange.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>

using kanji_tools::Args, kanji_tools::KanjiRange, kanji_tools::KanaRange,
    kanji_tools::fromUtf8ToWstring;

void format(Args args) {
  if (args.size() < 2) throw std::domain_error{"specify a file to format"};
  const auto* const file{args[1]};
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
}

int main(int argc, const char** argv) {
  try {
    format({argc, argv});
  } catch (const std::exception& err) {
    std::cerr << err.what() << '\n';
    return 1;
  }
  return 0;
}
