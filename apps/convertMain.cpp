#include <kanji/Kana.h>
#include <kanji/KanaConvert.h>

#include <filesystem>
#include <iostream>
#include <vector>

#include <stdio.h>
#include <unistd.h>

using namespace kanji;
namespace fs = std::filesystem;

class ConvertMain {
public:
  ConvertMain(int argc, const char** argv);
  void run();
private:
  void processOneLine(const std::string& s) {
    if (_source.has_value())
      std::cout << _converter.convert(s, *_source);
    else
      std::cout << _converter.convert(s);
  }
  void setFlag(int value) { _converter.flags(_converter.flags() | value); }
  bool charTypeArgs(const std::string& arg) {
    if (arg == "-h")
      _converter.target(CharType::Hiragana);
    else if (arg == "-k")
      _converter.target(CharType::Katakana);
    else if (arg == "-r")
      _converter.target(CharType::Romaji);
    else if (arg == "-H")
      _source = CharType::Hiragana;
    else if (arg == "-K")
      _source = CharType::Katakana;
    else if (arg == "-R")
      _source = CharType::Romaji;
    else
      return false;
    return true;
  }
  bool flagArgs(const std::string& arg) {
    if (arg == "h")
      setFlag(KanaConvert::Hepburn);
    else if (arg == "k")
      setFlag(KanaConvert::Kunrei);
    else if (arg == "n")
      setFlag(KanaConvert::NoProlongMark);
    else if (arg == "r")
      setFlag(KanaConvert::RemoveSpaces);
    else
      return false;
    return true;
  }
  std::optional<CharType> _source = std::nullopt;
  bool _interactive = false;
  bool _suppressNewLine = false;
  std::vector<std::string> _strings;
  KanaConvert _converter;
};

ConvertMain::ConvertMain(int argc, const char** argv) {
  auto usage = [&argc, &argv](const auto& s) {
    std::string program = argc > 0 ? fs::path(argv[0]).filename().string() : std::string("kanaConvert");
    std::cerr << program << ": " << s << "\nusage: " << program
              << " [-h|-k|-r] [-H|-K|-R] [-f h|n|r] [-i|-n] [string ...]\n\
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
  -n: suppress newline on output (for non-interactive mode)\n\
  --: finish parsing options, all further arguments will be treated as input files\n\
  [string ...]: provide one or more strings to convert, no strings means process standard input\n";
    exit(1);
  };
  bool finishedOptions = false;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (finishedOptions)
      _strings.push_back(arg);
    else if (arg == "--")
      finishedOptions = true; // any more args will be added to 'files'
    else if (arg == "-i")
      _interactive = true;
    else if (arg == "-n")
      _suppressNewLine = true;
    else if (arg == "-f") {
      if (i + 1 < argc) {
        ++i;
        arg = argv[i];
        if (!flagArgs(arg)) usage("unrecognized option for -f: " + arg);
      } else
        usage("-f must be followed by a flag value");
      } else if (arg.starts_with("-")) {
        if (!charTypeArgs(arg)) usage("unrecognized option: " + arg);
      } else
        _strings.push_back(arg);
  }
  if (_interactive && _suppressNewLine) usage("can't combine '-i' and '-n'");
  if (_strings.empty()) {
    if (isatty(fileno(stdin)) && !_interactive)
      usage("either provide one or more 'strings' to convert or specify '-i' for interactive mode");
  } else if (_interactive)
    usage("'-i' can't be combined with other 'string' arguments");
}

void ConvertMain::run() {
  if (std::string line; _strings.empty()) {
    do {
      if (_interactive) {
        std::cout << "Enter a string to convert";
        if (_source.has_value()) std::cout << " from " << toString(*_source);
        std::cout << " to " << toString(_converter.target()) << " or 'q' to quit:\n";
      }
      if (std::getline(std::cin, line) && line != "q") {
        processOneLine(line);
        if (!_suppressNewLine) std::cout << '\n';
      } else
        break;
    } while (true);
  } else {
    bool outputSpace = false;
    for (const auto& i : _strings) {
      if (outputSpace)
        std::cout << (_converter.target() == CharType::Romaji ? " " : "　");
      else
        outputSpace = _converter.target() != CharType::Romaji && !(_converter.flags() & KanaConvert::RemoveSpaces);
      processOneLine(i);
    }
    std::cout << '\n';
  }
}

int main(int argc, const char** argv) {
  ConvertMain convertMain(argc, argv);
  try {
    convertMain.run();
  } catch (const std::exception& err) {
    std::cerr << err.what() << '\n';
    return 1;
  }
  return 0;
}
