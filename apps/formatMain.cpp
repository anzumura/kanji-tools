#include <kt_utils/Args.h>
#include <kt_utils/BlockRange.h>
#include <kt_utils/Exception.h>

#include <filesystem>
#include <fstream>
#include <regex>

using kanji_tools::Args, kanji_tools::DomainError, kanji_tools::KanjiRange,
    kanji_tools::KanaRange, kanji_tools::fromUtf8ToWstring, kanji_tools::String;

void format(Args args) {
  if (args.size() < 2) throw DomainError{"specify a file to format"};
  const std::filesystem::path file{args[1]};
  if (!std::filesystem::is_regular_file(file))
    throw DomainError("file not found: " + file.string());
  const std::wregex endsWithKanji{std::wstring{L"["} + KanjiRange() + L"]{1}$"},
      allKana{std::wstring{L"^["} + KanaRange() + L"]+$"};
  std::fstream f{file};
  String line, prevLine;
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
