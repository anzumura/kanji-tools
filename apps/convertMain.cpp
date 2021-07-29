#include <kanji/Choice.h>
#include <kanji/Kana.h>
#include <kanji/KanaConvert.h>
#include <kanji/MBUtils.h>
#include <kanji/Table.h>

#include <filesystem>

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
  void printKanaChart(bool markdown = false) const;

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
  bool finishedOptions = false, printKana = false, printMarkdown = false;
  auto setBool = [this, &printKana, &printMarkdown](bool& b) {
    if (_interactive || _suppressNewLine || printKana || printMarkdown)
      usage("Can only specify one of -i, -m, -n, or -p");
    b = true;
  };
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (finishedOptions)
      _strings.push_back(arg);
    else if (arg == "--")
      finishedOptions = true; // any more args will be added to 'files'
    else if (arg == "-i")
      setBool(_interactive);
    else if (arg == "-m")
      setBool(printMarkdown);
    else if (arg == "-n")
      setBool(_suppressNewLine);
    else if (arg == "-p")
      setBool(printKana);
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
  if (_strings.empty()) {
    if (isatty(fileno(stdin))) {
      if (printKana) printKanaChart();
      if (printMarkdown)
        printKanaChart(true);
      else if (!_interactive)
        usage("provide one or more 'strings' to convert or specify '-i' for interactive mode");
    }
  } else if (_interactive)
    usage("'-i' can't be combined with other 'string' arguments");
}

void ConvertMain::usage(const std::string& errorMsg, bool showAllOptions) const {
  std::ostream& os = errorMsg.empty() ? std::cout : std::cerr;
  if (!errorMsg.empty()) os << _program << ": " << errorMsg << '\n';
  if (showAllOptions) os << "usage: " << _program << " [-h|-k|-r] [-H|-K|-R] [-f h|n|r] [-i|-m|-n|-p] [string ...]\n";
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
  -m: print kana chart in 'Markdown' format and exit\n\
  -n: suppress newline on output (for non-interactive mode)\n\
  -p: print kana chart aligned for terminal output and exit\n\
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

void ConvertMain::printKanaChart(bool markdown) const {
  std::cout << (markdown ? "**Notes:**" : ">>> Notes:");
  std::cout << "\n\
- Roma=Rōmaji, Hira=Hiragana, Kata=Katakana, Uni=Unicode, Hepb=Hepburn, Kunr=Kunrei\n\
- Roma is mainly 'Modern Hepburn', but can be 'Nihon Shiki' or 'Wāpuro' in some cases\n\
- Hepb and Kunr are only populated when they would produce different output\n\
  - Values in () means 'output-only' since inputting leads to a different kana\n\
- 'Roma Variants' are alternative keyboard combinations that lead to the same kana\n\
- When populated, Roma, Hira and Kata columns are unique (no duplicates)\n\
- Unicode values are only shown for 'monograph' entries\n\
- Some 'digraphs' may not be in any real words, but they are typable and thus included\n\
- Chart output is sorted by Hiragana, so 'a, ka, sa, ta, na, ...' ordering\n\
- Katakana 'dakuten w' (ヷ, ヸ, ヹ, ヺ) aren't suppoted (no standard Hiragana or Romaji)\n\
- Type values: P=Plain Kana, D=Dakuten, H=HanDakuten, N=None\n\
- Type 'N' includes:\n\
  - Middle Dot/Interpunct (・): maps to Rōmaji '/' to match usual IME keyboard entry\n\
  - Prolong Mark (ー): conversion via macrons (ā, ī, ū, ē, ō) so no single Rōmaji value\n\
  - Repeat symbols (ゝ, ゞ, ヽ, ヾ): conversion only supported when 'target' is Rōmaji\n\n";
  int hanDakutenMonographs = 0, small = 0, plainMonographs = 0, dakutenMonographs = 0, plainDigraphs = 0,
      hanDakutenDigraphs = 0, dakutenDigraphs = 0, romajiVariants = 0;
  Table table({"No.", "Type", "Roma", "Hira", "Kata", "HUni", "KUni", "Hepb", "Kunr", "Roma Variants"}, true);
  const std::string empty;
  // Put a border before each 'group' of kana - use 'la', 'lya' and 'lwa' when there are small letters
  // that should be included, i.e., 'la' (ぁ) comes right before 'a' (あ).
  std::set<std::string> groups{"la", "ka", "sa", "ta", "na", "ha", "ma", "lya", "ra", "lwa"};
  for (auto& entry : Kana::getMap(CharType::Hiragana)) {
    auto& i = *entry.second;
    romajiVariants += i.romajiVariants().size();
    if (i.isSmall())
      ++small;
    else if (i.isMonograph()) {
      if (i.isDakuten())
        ++dakutenMonographs;
      else if (i.isHanDakuten())
        ++hanDakutenMonographs;
      else
        ++plainMonographs;
    } else {
      if (i.isDakuten())
        ++dakutenDigraphs;
      else if (i.isHanDakuten())
        ++hanDakutenDigraphs;
      else
        ++plainDigraphs;
    }
    const std::string type(i.isDakuten() ? "D" : i.isHanDakuten() ? "H" : "P");
    const std::string& romaji = i.romaji();
    const std::string& h = i.hiragana();
    const std::string& k = i.katakana();
    std::string hepb(i.getRomaji(KanaConvert::Hepburn));
    std::string kunr(i.getRomaji(KanaConvert::Kunrei));
    hepb = romaji == hepb ? empty : ('(' + hepb + ')');
    kunr = romaji == kunr ? empty : i.kunreiVariant() ? kunr : ('(' + kunr + ')');
    std::string vars;
    for (int j = (i.kunreiVariant() ? 1 : 0); j < i.romajiVariants().size(); ++j) {
      if (!vars.empty()) vars += ", ";
      vars += i.romajiVariants()[j];
    }
    // only show unicode for monographs
    auto uni = [&i, &empty](auto& s) { return i.isMonograph() ? toUnicode(s) : empty; };
    table.add({type, romaji, h, k, uni(h), uni(k), hepb, kunr, vars}, groups.contains(romaji));
  }
  // special handling middle dot, prolong symbol and repeat symbols
  const char slash = '/';
  const auto& middleDot = _converter.narrowDelims().find(slash);
  // middleDot should always be found and thus '4' none rows, but handle if missing just in case ...
  const int none = middleDot != _converter.narrowDelims().end() ? 4 : 3;
  if (none == 4)
    table.add({"N", empty + slash, empty, middleDot->second, empty, toUnicode(middleDot->second)}, true);
  else
    std::cerr << "Failed to find " << slash << " in _converter.narrowDelims()\n";
  table.add({"N", empty, empty, Kana::ProlongMark, empty, toUnicode(Kana::ProlongMark)}, none == 3);
  for (auto& i : std::array{&Kana::RepeatPlain, &Kana::RepeatAccented}) {
    const std::string& h = i->hiragana();
    const std::string& k = i->katakana();
    table.add({"N", empty, h, k, toUnicode(h), toUnicode(k)});
  }
  markdown ? table.printMarkdown() : table.print();
  const int monographs = small + plainMonographs + dakutenMonographs + hanDakutenMonographs;
  const int digraphs = plainDigraphs + dakutenDigraphs + hanDakutenDigraphs;
  const int plain = small + plainMonographs + plainDigraphs;
  const int dakuten = dakutenMonographs + dakutenDigraphs;
  const int hanDakuten = hanDakutenMonographs + hanDakutenDigraphs;
  const int types = plain + dakuten + hanDakuten + none;
  auto out = [markdown](const std::string& s) -> std::ostream& {
    if (markdown)
      std::cout << "- ";
    else
      std::cout << std::setw(10);
    return std::cout << s << ": " << std::setw(3);
  };
  std::cout << '\n' << (markdown ? "**Totals:**" : ">>> Totals:") << std::setfill(' ') << std::right << '\n';
  out("Monograph") << monographs << " (Plain=" << plainMonographs << ", Dakuten=" << dakutenMonographs
                   << ", HanDakuten=" << hanDakutenMonographs << ", Small=" << small << ")\n";
  out("Digraphs") << digraphs << " (Plain=" << plainDigraphs << ", Dakuten=" << dakutenDigraphs
                  << ", HanDakuten=" << hanDakutenDigraphs << ")\n";
  out("All Kana") << monographs + digraphs << " (Monographs=" << monographs << ", Digraphs=" << digraphs
                  << "), Rōmaji Variants=" << romajiVariants << '\n';
  out("Types") << types << " (P=" << plain << ", D=" << dakuten << ", H=" << hanDakuten << ", N=" << none
               << "), N types are not included in 'All Kana'\n";
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
