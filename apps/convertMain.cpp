#include <kanji/Choice.h>
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
  // 'usage' prints optional error message and then details about valid arguments. If 'showAllOptions' is
  // true then all options are displayed and the program exits.
  void usage(const std::string& errorMsg = "", bool showAllOptions = true) const;
  void processOneLine(const std::string& s) {
    std::cout << (_source.has_value() ? _converter.convert(*_source, s) : _converter.convert(s));
  }
  void getInput();
  void setFlag(int value) { _converter.flags(_converter.flags() | value); }
  bool charTypeArgs(const std::string& arg);
  bool flagArgs(char arg);
  void printKanaChart() const;

  bool _interactive = false;
  bool _suppressNewLine = false;
  std::optional<CharType> _source = std::nullopt;
  std::vector<std::string> _strings;
  Choice _choice;
  KanaConvert _converter;
  const std::string _program;
};

ConvertMain::ConvertMain(int argc, const char** argv)
  : _choice(std::cout), _program(argc > 0 ? fs::path(argv[0]).filename().string() : std::string("kanaConvert")) {
  bool finishedOptions = false, printKana = false;
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
    else if (arg == "-p")
      printKana = true;
    else if (arg == "-?")
      usage();
    else if (arg == "-f") {
      if (i + 1 < argc) {
        ++i;
        arg = argv[i];
        if (arg.length() != 1 || !flagArgs(arg[0])) usage("illegal option for -f: " + arg);
      } else
        usage("-f must be followed by a flag value");
    } else if (arg.starts_with("-")) {
      if (!charTypeArgs(arg)) usage("illegal option: " + arg);
    } else
      _strings.push_back(arg);
  }
  if (_interactive && _suppressNewLine) usage("can't combine '-i' and '-n'");
  if (_interactive && printKana) usage("can't combine '-i' and '-p'");
  if (printKana && _suppressNewLine) usage("can't combine '-p' and '-n'");
  if (_strings.empty()) {
    if (isatty(fileno(stdin))) {
      if (printKana)
        printKanaChart();
      else if (!_interactive)
        usage("provide one or more 'strings' to convert or specify '-i' for interactive mode");
    }
  } else if (_interactive)
    usage("'-i' can't be combined with other 'string' arguments");
}

void ConvertMain::usage(const std::string& errorMsg, bool showAllOptions) const {
  std::ostream& os = errorMsg.empty() ? std::cout : std::cerr;
  if (!errorMsg.empty()) os << _program << ": " << errorMsg << '\n';
  if (showAllOptions) os << "usage: " << _program << " [-h|-k|-r] [-H|-K|-R] [-f h|n|r] [-i|-n|-p] [string ...]\n";
  os << "  -h: set conversion output to Hiragana" << (showAllOptions ? " (default)" : "") << "\n\
  -k: set conversion output to Katakana\n\
  -r: set conversion output to Romaji\n\
  -H: restrict conversion input to Hiragana\n\
  -K: restrict conversion input to Katakana\n\
  -R: restrict conversion input to Romaji\n";
  if (showAllOptions) {
    os << "\
  -?: prints this usage message\n\
  -f option: set 'option' (-f can be used multiple times to combine options). Valid options are:\n\
      h: conform Romaji output more closely to 'Modern Hepburn' style\n\
      k: conform Romaji output more closely to 'Kunrei Shiki' style\n\
      n: no prolonged sound marks on Hiragana output, i.e., vowels are repeated instead of 'ー'\n\
      r: remove spaces on output (only applies to Hiragana and Katakana output)\n\
  -i: interactive mode\n\
  -n: suppress newline on output (for non-interactive mode)\n\
  -p: print kana chart and exit\n\
  --: finish parsing options, all further arguments will be treated as input files\n\
  [string ...]: provide one or more strings to convert, no strings means process standard input\n";
    exit(errorMsg.empty() ? 0 : 1);
  }
}

void ConvertMain::run() {
  if (std::string line; _strings.empty())
    getInput();
  else {
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

void ConvertMain::getInput() {
  static Choice::Choices flagChoices{{'h', "Hepburn"}, {'k', "Kunrei"}, {'n', "NoProlongMark"}, {'r', "RemoveSpaces"}};
  bool outputCurrentOptions = true;
  do {
    if (_interactive && outputCurrentOptions) {
      std::cout << ">>> Current options: source=" << (_source.has_value() ? toString(*_source) : "any")
                << ", target=" << toString(_converter.target()) << ", flags=" << _converter.flagString()
                << "\n>>> Enter string or 'c'=clear flags, 'f'=set flag, 'q'=quit, 'h'=help or -k|-h|-r|-K|-H|-R:\n";
      outputCurrentOptions = false;
    }
    if (std::string line; std::getline(std::cin, line) && line != "q") {
      if (_interactive) {
        if (line.empty()) continue;
        if (line == "c" || line == "f" || line == "h" || line.starts_with("-")) {
          if (line == "c")
            _converter.flags(0);
          else if (line == "f")
            flagArgs(_choice.get(">>> Enter flag option", flagChoices));
          else if (line == "h")
            usage("", false);
          else if (line.starts_with("-")) {
            if (!charTypeArgs(line)) std::cout << "  illegal option: " << line << '\n';
          }
          outputCurrentOptions = true;
          continue;
        }
      }
      processOneLine(line);
      if (!_suppressNewLine) std::cout << '\n';
    } else
      break;
  } while (true);
}

bool ConvertMain::charTypeArgs(const std::string& arg) {
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

bool ConvertMain::flagArgs(char arg) {
  if (arg == 'h')
    setFlag(KanaConvert::Hepburn);
  else if (arg == 'k')
    setFlag(KanaConvert::Kunrei);
  else if (arg == 'n')
    setFlag(KanaConvert::NoProlongMark);
  else if (arg == 'r')
    setFlag(KanaConvert::RemoveSpaces);
  else
    return false;
  return true;
}

void ConvertMain::printKanaChart() const {
  std::cout << "Printing Kana Chart\n\
  - meaning of Type values: K=Kana, D=Dakuten, H=HanDakuten\n\
  - a few 'K' type Kana are actually 'Dakuten Digraphs' without a non-accented version\n\
  - any Hepburn or Kunrei values in brackets () are 'output-only' since they are ambiguous\n\n";
  auto print = [](const std::string& c1, const std::string& c2, const std::string& c3, const std::string& c4,
                  const std::string& c5, const std::string& c6, const std::string& c7, const std::string& c8) {
    // for the title row leave 2 spaces (after Hiragana and Katakana) and for kana rows leave 8 spaces
    // for a single or 6 spaces for a digraph (since each kana is twice as wide as an ascii character)
    std::string spaces(c4.length() == 8 ? "  " : c4.length() == 3 ? "        " : "      ");
    std::cout << std::left << std::setw(5) << c1 << std::setw(6) << c2 << std::setw(10) << c3 << c4 << spaces << c5
              << spaces << std::setw(10) << c6 << std::setw(10) << c7 << c8 << '\n';
  };
  print("No.", "Type", "Romaji", "Hiragana", "Katakana", "Hepburn", "Kunrei", "Variants");
  int count = 0;
  std::string empty;
  for (auto& i : Kana::getMap(CharType::Hiragana)) {
    auto& k = *i.second;
    std::string type(k.isDakuten() ? "D" : k.isHanDakuten() ? "H" : "K");
    std::string variants;
    for (int j = (k.kunreiVariant() ? 1 : 0); j < k.variants().size(); ++j) {
      if (!variants.empty()) variants += ", ";
      variants += k.variants()[j];
    }
    std::string hepburn(k.getRomaji(KanaConvert::Hepburn));
    std::string kunrei(k.getRomaji(KanaConvert::Kunrei));
    const std::string& r = k.romaji();
    print(std::to_string(++count), type, r, k.hiragana(), k.katakana(), r == hepburn ? empty : ('(' + hepburn + ')'),
          r == kunrei           ? empty
            : k.kunreiVariant() ? kunrei
                                : ('(' + kunrei + ')'),
          variants);
  }
  exit(0);
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
