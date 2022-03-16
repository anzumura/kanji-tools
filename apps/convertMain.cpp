#include <kanji_tools/kana/KanaConvert.h>
#include <kanji_tools/kana/Table.h>
#include <kanji_tools/utils/Choice.h>
#include <kanji_tools/utils/MBUtils.h>
#include <stdio.h>
#include <unistd.h>

#include <filesystem>

using namespace kanji_tools;
namespace fs = std::filesystem;

class ConvertMain {
public:
  ConvertMain(int argc, const char** argv);
  void run();
private:
  // 'usage' prints optional error message and then details about valid
  // arguments. If 'showAllOptions' is true then all options are displayed and
  // the program exits.
  void usage(const std::string& errorMsg = Table::EmptyString,
             bool showAllOptions = true) const;
  void processOneLine(const std::string& s) {
    std::cout << (_source ? _converter.convert(*_source, s)
                          : _converter.convert(s));
  }
  void getInput();
  void setFlag(ConvertFlags value) {
    _converter.flags(_converter.flags() | value);
  }
  bool charTypeArgs(const std::string& arg);
  bool flagArgs(char arg);
  void printKanaChart(bool markdown = false) const;

  bool _interactive{false};
  bool _suppressNewLine{false};
  std::optional<CharType> _source{};
  std::vector<std::string> _strings;
  KanaConvert _converter;
  const Choice _choice;
  const std::string _program;
};

ConvertMain::ConvertMain(int argc, const char** argv)
    : _choice(std::cout),
      _program(argc > 0 ? fs::path(argv[0]).filename().string()
                        : std::string("kanaConvert")) {
  auto finishedOptions{false}, printKana{false}, printMarkdown{false};
  const auto setBool{[this, &printKana, &printMarkdown](bool& b) {
    if (_interactive || _suppressNewLine || printKana || printMarkdown)
      usage("Can only specify one of -i, -m, -n, or -p");
    b = true;
  }};
  for (auto i{1}; i < argc; ++i) {
    std::string arg{argv[i]};
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
        if (arg.size() != 1 || !flagArgs(arg[0]))
          usage("illegal option for -f: " + arg);
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
        usage("provide one or more 'strings' to convert or specify '-i' for "
              "interactive mode");
    }
  } else if (_interactive)
    usage("'-i' can't be combined with other 'string' arguments");
}

void ConvertMain::usage(const std::string& errorMsg,
                        bool showAllOptions) const {
  auto& os{errorMsg.empty() ? std::cout : std::cerr};
  if (!errorMsg.empty()) os << _program << ": " << errorMsg << '\n';
  if (showAllOptions)
    os << "usage: " << _program
       << " [-h|-k|-r] [-H|-K|-R] [-f h|n|r] [-i|-m|-n|-p] [string ...]\n";
  os << "  -h: set conversion output to Hiragana"
     << (showAllOptions ? " (default)" : "") << R"(
  -k: set conversion output to Katakana
  -r: set conversion output to Romaji
  -H: restrict conversion input to Hiragana
  -K: restrict conversion input to Katakana
  -R: restrict conversion input to Romaji
)";
  if (showAllOptions) {
    os << R"(  -?: prints this usage message
  -f option: set 'option' (-f can be used multiple times to combine options). Valid options are:
      h: conform Romaji output more closely to 'Modern Hepburn' style
      k: conform Romaji output more closely to 'Kunrei Shiki' style
      n: no prolonged sound marks on Hiragana output, i.e., vowels are repeated instead of 'ー'
      r: remove spaces on output (only applies to Hiragana and Katakana output)
  -i: interactive mode
  -m: print kana chart in 'Markdown' format and exit
  -n: suppress newline on output (for non-interactive mode)
  -p: print kana chart aligned for terminal output and exit
  --: finish parsing options, all further arguments will be treated as input files
  [string ...]: provide one or more strings to convert, no strings means process standard input
)";
    exit(errorMsg.empty() ? 0 : 1);
  }
}

void ConvertMain::run() {
  if (std::string line; _strings.empty())
    getInput();
  else {
    for (auto space{false}; const auto& i : _strings) {
      if (space)
        std::cout << (_converter.target() == CharType::Romaji ? " " : "　");
      else
        space = _converter.target() != CharType::Romaji &&
                !(_converter.flags() & ConvertFlags::RemoveSpaces);
      processOneLine(i);
    }
    std::cout << '\n';
  }
}

void ConvertMain::getInput() {
  static Choice::Choices flagChoices{{'h', "Hepburn"},
                                     {'k', "Kunrei"},
                                     {'n', "NoProlongMark"},
                                     {'r', "RemoveSpaces"}};
  auto outputCurrentOptions{true};
  do {
    if (_interactive && outputCurrentOptions) {
      std::cout << ">>> Current options: source="
                << (_source ? toString(*_source) : "any")
                << ", target=" << toString(_converter.target())
                << ", flags=" << _converter.flagString()
                << "\n>>> Enter string or 'c'=clear flags, 'f'=set flag, "
                   "'q'=quit, 'h'=help or -k|-h|-r|-K|-H|-R:\n";
      outputCurrentOptions = false;
    }
    if (std::string line; std::getline(std::cin, line) && line != "q") {
      if (_interactive) {
        if (line.empty()) continue;
        if (line == "c" || line == "f" || line == "h" ||
            line.starts_with("-")) {
          if (line == "c")
            _converter.flags(ConvertFlags::None);
          else if (line == "f")
            flagArgs(_choice.get(">>> Enter flag option", flagChoices));
          else if (line == "h")
            usage("", false);
          else if (line.starts_with("-")) {
            if (!charTypeArgs(line))
              std::cout << "  illegal option: " << line << '\n';
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
    setFlag(ConvertFlags::Hepburn);
  else if (arg == 'k')
    setFlag(ConvertFlags::Kunrei);
  else if (arg == 'n')
    setFlag(ConvertFlags::NoProlongMark);
  else if (arg == 'r')
    setFlag(ConvertFlags::RemoveSpaces);
  else
    return false;
  return true;
}

void ConvertMain::printKanaChart(bool markdown) const {
  std::cout << (markdown ? "## **Kana Conversion Chart**\n### **Notes:**"
                         : ">>> Notes:");
  std::cout << R"(
- Abbreviations used below: Roma=Rōmaji, Hira=Hiragana, Kata=Katakana,
                            Uni=Unicode, Hepb=Hepburn, Kunr=Kunrei
- Roma is mainly 'Modern Hepburn', but can be 'Nihon Shiki' or 'Wāpuro'
- Hepb and Kunr are only populated when they would produce different output
  - Values in () means 'output-only' since inputting leads to a different kana
- 'Roma Variants' are alternative key combinations that lead to the same kana
- When populated, Roma, Hira and Kata columns are unique (no duplicates)
- Unicode values are only shown for 'monograph' entries
- Some 'digraphs' may not be in any real words, but include for completeness
- Chart output is sorted by Hiragana, so 'a, ka, sa, ta, na, ...' ordering
- Katakana 'dakuten w' (ヷ, ヸ, ヹ, ヺ) aren't suppoted (no conversion exist)
- Type values: P=Plain Kana, D=Dakuten, H=HanDakuten, N=None
- Type 'N' includes:
  - Middle Dot/Interpunct (・): maps to Rōmaji '/' to match IME keyboard entry
  - Prolong Mark (ー): convert to/from macrons (ā, ī, ū, ē, ō)
  - Repeat symbols (ゝ, ゞ, ヽ, ヾ): only supported when 'target' is Rōmaji
)";
  u_int8_t hanDakutenMonographs{}, small{}, plainMonographs{},
    dakutenMonographs{}, plainDigraphs{}, hanDakutenDigraphs{},
    dakutenDigraphs{}, romajiVariants{};
  Table table{{"No.", "Type", "Roma", "Hira", "Kata", "HUni", "KUni", "Hepb",
               "Kunr", "Roma Variants"},
              true};
  const std::string empty;
  // Put a border before each 'group' of kana - use 'la', 'lya' and 'lwa' when
  // there are small letters that should be included, i.e., 'la' (ぁ) comes
  // right before 'a' (あ).
  const std::set<std::string> groups{"la", "ka", "sa",  "ta", "na",
                                     "ha", "ma", "lya", "ra", "lwa"};
  for (auto& entry : Kana::getMap(CharType::Hiragana)) {
    auto& i{*entry.second};
    romajiVariants += static_cast<u_int8_t>(i.romajiVariants().size());
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
    const std::string type{i.isDakuten() ? "D" : i.isHanDakuten() ? "H" : "P"};
    auto& romaji{i.romaji()};
    auto& h{i.hiragana()};
    auto& k{i.katakana()};
    std::string hepb{i.getRomaji(ConvertFlags::Hepburn)};
    std::string kunr{i.getRomaji(ConvertFlags::Kunrei)};
    hepb = romaji == hepb ? empty : ('(' + hepb + ')');
    kunr = romaji == kunr      ? empty
           : i.kunreiVariant() ? kunr
                               : ('(' + kunr + ')');
    std::string vars;
    for (auto j : i.romajiVariants()) {
      if (!vars.empty()) vars += ", ";
      vars += j;
    }
    // only show unicode for monographs
    const auto uni{
      [&i, &empty](auto& s) { return i.isMonograph() ? toUnicode(s) : empty; }};
    table.add({type, romaji, h, k, uni(h), uni(k), hepb, kunr, vars},
              groups.contains(romaji));
  }
  // special handling middle dot, prolong symbol and repeat symbols
  const auto slash{'/'};
  const auto& middleDot{_converter.narrowDelims().find(slash)};
  // middleDot should always be found and thus '4' none rows, but handle if
  // missing just in case ...
  static constexpr u_int8_t HasSlash{4}, NoSlash{3};
  const u_int8_t none{middleDot != _converter.narrowDelims().end() ? HasSlash
                                                                   : NoSlash};
  if (none == HasSlash)
    table.add({"N", empty + slash, empty, middleDot->second, empty,
               toUnicode(middleDot->second)},
              true);
  else
    std::cerr << "Failed to find " << slash
              << " in _converter.narrowDelims()\n";
  table.add(
    {"N", empty, empty, Kana::ProlongMark, empty, toUnicode(Kana::ProlongMark)},
    none == NoSlash);
  for (auto& i : std::array{&Kana::RepeatPlain, &Kana::RepeatAccented}) {
    auto& h{i->hiragana()};
    auto& k{i->katakana()};
    table.add({"N", empty, h, k, toUnicode(h), toUnicode(k)});
  }
  markdown ? table.printMarkdown() : table.print();
  const auto monographs{small + plainMonographs + dakutenMonographs +
                        hanDakutenMonographs},
    digraphs{plainDigraphs + dakutenDigraphs + hanDakutenDigraphs},
    plain{small + plainMonographs + plainDigraphs},
    dakuten{dakutenMonographs + dakutenDigraphs},
    hanDakuten{hanDakutenMonographs + hanDakutenDigraphs};
  const auto types{plain + dakuten + hanDakuten + none};
  const auto out{[markdown](const std::string& s) -> std::ostream& {
    if (markdown)
      std::cout << "- **";
    else
      std::cout << std::setw(10);
    return std::cout << s << (markdown ? ":** " : ": ") << std::setw(3);
  }};
  std::cout << '\n'
            << (markdown ? "### **Totals:**" : ">>> Totals:")
            << std::setfill(' ') << std::right << '\n';
  out("Monograph") << monographs << " (Plain=" << plainMonographs
                   << ", Dakuten=" << dakutenMonographs
                   << ", HanDakuten=" << hanDakutenMonographs
                   << ", Small=" << small << ")\n";
  out("Digraphs") << digraphs << " (Plain=" << plainDigraphs
                  << ", Dakuten=" << dakutenDigraphs
                  << ", HanDakuten=" << hanDakutenDigraphs << ")\n";
  out("All Kana") << monographs + digraphs << " (Monographs=" << monographs
                  << ", Digraphs=" << digraphs
                  << "), Rōmaji Variants=" << romajiVariants << '\n';
  out("Types") << types << " (P=" << plain << ", D=" << dakuten
               << ", H=" << hanDakuten << ", N=" << none
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
