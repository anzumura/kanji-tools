#include <kanji/Choice.h>
#include <kanji/Kana.h>
#include <kanji/KanaConvert.h>
#include <kanji/MBUtils.h>

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
  std::cout << "--- Kana Chart ---\n\
Columns: 'Roma'=Rōmaji, 'Hira'=Hiragana, 'Kata'=Katakana, 'HUni'=Hiragana Unicode\n\
         'KUni'=Katakana Unicode, 'Hepb'=Hepburn, 'Kunr'=Kunrei, 'Vars'=Variants\n\
Types: 'K'=Kana, 'D'=Dakuten, 'H'=HanDakuten, 'N'=None - type 'N' includes:\n\
- Middle Dot (・): maps to Rōmaji '/' to match usual IME keyboard entry\n\
- Prolong Mark (ー): conversion via macrons (ā, ī, ū, ē, ō) so no single Rōmaji value\n\
- Repeat symbols (ゝ, ゞ, ヽ, ヾ): conversion only supported when 'target' is Rōmaji\n\
Notes:\n\
- When populated, 'Roma', 'Hira' and 'Kata' columns are unique (no duplicates)\n\
- 'Roma' is mainly 'Modern Hepburn', but can be 'Nihon Shiki' or 'Wāpuro' in some cases\n\
- 'Hepb' and 'Kunr' in () means 'output-only' since inputting leads to a different kana\n\
- 'Vars' are alternative keyboard combinations that lead to the same kana\n\
- Some 'K' types are actually 'Dakuten Digraphs' with no unaccented version\n\
- Unicode values are only shown for 'monograph' entries\n\
- Some 'digraphs' may not be in any real words, but they are typable and thus included\n\
- Chart output is sorted on 'Hira' column, so 'a, ka, sa, ta, na, ...' ordering\n\
- Katakana 'dakuten w' (ヷ, ヸ, ヹ, ヺ) aren't suppoted (no standard Hiragana or Romaji)\n\
- 'Hepb' and 'Kunr' are only populated when they would produce different output\n\n";
  auto print = [](const std::string& num, const std::string& type, const std::string& roma, const std::string& hira,
                  const std::string& kata, const std::string& hUni, const std::string& kUni,
                  const std::string& hepb = "", const std::string& kunr = "", const std::string& vars = "") {
    // Leave 2 spaces after titles (Hira and Kata) and 'digraph' kana and 4 spaces after a single (since
    // each kana is twice as wide as an ascii character). Use 6 spaces is 'empty'.
    auto sp = [](auto& s) { return s.length() > 3 ? "  " : s.empty() ? "      " : "    "; };
    std::cout << std::left << std::setw(5) << num << std::setw(6) << type << std::setw(6) << roma << hira << sp(hira)
              << kata << sp(kata) << std::setw(6) << hUni << std::setw(6) << kUni << std::setw(6) << hepb
              << std::setw(6) << kunr << vars << '\n';
  };
  print("No.", "Type", "Roma", "Hira", "Kata", "HUni", "KUni", "Hepb", "Kunr", "Vars");
  int row = 0, monographs = 0, digraphs = 0, variants = 0, kana = 0, dakuten = 0, hanDakuten = 0, none = 0;
  std::string empty;
  for (auto& entry : Kana::getMap(CharType::Hiragana)) {
    auto& i = *entry.second;
    std::string t(i.isDakuten() ? (++dakuten, "D") : i.isHanDakuten() ? (++hanDakuten, "H") : (++kana, "K"));
    variants += i.variants().size();
    std::string vars;
    for (int j = (i.kunreiVariant() ? 1 : 0); j < i.variants().size(); ++j) {
      if (!vars.empty()) vars += ", ";
      vars += i.variants()[j];
    }
    std::string hepb(i.getRomaji(KanaConvert::Hepburn));
    std::string kunr(i.getRomaji(KanaConvert::Kunrei));
    const std::string& h = i.hiragana();
    const std::string& k = i.katakana();
    const std::string& r = i.romaji();
    const bool uni = h.length() == 3; // only show unicode for monographs
    if (uni)
      ++monographs;
    else
      ++digraphs;
    hepb = r == hepb ? empty : ('(' + hepb + ')');
    kunr = r == kunr ? empty : i.kunreiVariant() ? kunr : ('(' + kunr + ')');
    print(std::to_string(++row), t, r, h, k, uni ? toUnicode(h) : empty, uni ? toUnicode(k) : empty, hepb, kunr, vars);
  }
  // special handling middle dot, prolong symbol and repeat symbols
  const char slash = '/';
  const auto& middleDot = _converter.narrowDelims().find(slash);
  if (middleDot != _converter.narrowDelims().end())
    print(std::to_string(++row), "N", empty + slash, empty, middleDot->second, empty, toUnicode(middleDot->second));
  else
    std::cerr << "Failed to find " << slash << " in _converter.narrowDelims()\n";
  print(std::to_string(++row), "N", empty, empty, Kana::ProlongMark, empty, toUnicode(Kana::ProlongMark));
  for (auto& i : std::array{Kana::RepeatUnaccented, Kana::RepeatAccented}) {
    const std::string& h = i.hiragana();
    const std::string& k = i.katakana();
    print(std::to_string(++row), "N", empty, h, k, toUnicode(h), toUnicode(k));
    ++none;
  }
  std::cout << "\nTotals: monographs=" << monographs << ", digraphs=" << digraphs << ", variants=" << variants
            << ", K=" << kana << ", D=" << dakuten << ", H=" << hanDakuten << ", N=" << none << '\n';
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
