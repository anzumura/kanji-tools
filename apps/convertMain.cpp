#include <kanji/KanaConvert.h>

#include <filesystem>
#include <iostream>
#include <vector>

using namespace kanji;
namespace fs = std::filesystem;

int main(int argc, const char** argv) {
  auto usage = [&argc, &argv](const auto& s) {
    std::string program = argc > 0 ? fs::path(argv[0]).filename().string() : std::string("kanaConvert");
    std::cerr << program << ": " << s << "\nusage: " << program
              << " [-h|-k|-r] [-H|-K|-R] [-f h|n|r] [-i] [string ...]\n\
  -h: set conversion output to Hiragana (default)\n\
  -k: set conversion output to Katakana\n\
  -r: set conversion output to Romaji\n\
  -H: restrict conversion input to Hiragana\n\
  -K: restrict conversion input to Katakana\n\
  -R: restrict conversion input to Romaji\n\
  -f option: set 'option' (-f can be used multiple times to combine options). Valid options are:\n\
      h: conform Romaji output more closely to 'Modern Hepburn' style\n\
      k: conform Romaji output more closely to 'Kunrei Shiki' style\n\
      n: no prolonged sound marks on Hiragana output, i.e., vowels are repeated instead of 'ー'\n\
      r: remove spaces on output (only applies to Hiragana and Katakana output)\n\
  -i: interactive mode\n\
  --: finish parsing options, all further arguments will be treated as input files\n\
  [string ...]: provide one or more strings to convert, no strings means process standard input\n";
    exit(1);
  };
  try {
    std::optional<CharType> source = std::nullopt;
    CharType target = CharType::Hiragana;
    int flags = 0;
    bool interactive = false, finishedOptions = false;
    std::vector<std::string> strings;
    for (int i = 1; i < argc; ++i) {
      std::string arg = argv[i];
      if (finishedOptions)
        strings.push_back(arg);
      else if (arg == "--")
        finishedOptions = true; // any more args will be added to 'files'
      else if (arg == "-i")
        interactive = true;
      else if (arg == "-h")
        target = CharType::Hiragana;
      else if (arg == "-k")
        target = CharType::Katakana;
      else if (arg == "-r")
        target = CharType::Romaji;
      else if (arg == "-H")
        source = CharType::Hiragana;
      else if (arg == "-K")
        source = CharType::Katakana;
      else if (arg == "-R")
        source = CharType::Romaji;
      else if (arg == "-f") {
        if (i + 1 < argc) {
          ++i;
          arg = argv[i];
          if (arg == "h")
            flags |= KanaConvert::Hepburn;
          else if (arg == "k")
            flags |= KanaConvert::Kunrei;
          else if (arg == "n")
            flags |= KanaConvert::NoProlongMark;
          else if (arg == "r")
            flags |= KanaConvert::RemoveSpaces;
          else
            usage("unrecognized option for -f: " + arg);
        } else
          usage("-f must be followed by a flag value");
      } else if (arg.starts_with("-"))
        usage("unrecognized option: " + arg);
      else
        strings.push_back(arg);
    }

    KanaConvert converter;
    bool outputSpace = false;
    auto process = [source, target, flags, &converter, &outputSpace](const auto& s) {
      if (outputSpace)
        std::cout << (target == CharType::Romaji ? " " : "　");
      else
        outputSpace = target != CharType::Romaji && !(flags & KanaConvert::RemoveSpaces);
      if (source.has_value())
        std::cout << converter.convert(s, *source, target, flags);
      else
        std::cout << converter.convert(s, target, flags);
    };
    if (std::string s; strings.empty())
      while (std::getline(std::cin, s))
        process(s);
    else
      for (const auto& i : strings)
        process(i);
    std::cout << '\n';
  } catch (const std::exception& err) {
    std::cerr << err.what() << '\n';
    return 1;
  }
  return 0;
}
