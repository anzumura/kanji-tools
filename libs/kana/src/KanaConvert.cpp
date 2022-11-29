#include <kt_kana/Kana.h>
#include <kt_kana/KanaConvert.h>
#include <kt_kana/Table.h>

#include <cstdio>
#include <unistd.h>

namespace kanji_tools {

namespace {

class KanaCount final {
public:
  void add(const Kana& i) {
    ++(i.isSmall()         ? _small
        : i.isDakuten()    ? _dakuten
        : i.isHanDakuten() ? _hanDakuten
                           : _plain);
  }

  [[nodiscard]] auto dakuten() const { return _dakuten; }
  [[nodiscard]] auto hanDakuten() const { return _hanDakuten; }
  [[nodiscard]] auto plain() const { return _plain; }
  [[nodiscard]] auto small() const { return _small; }

  [[nodiscard]] auto total() const {
    return _dakuten + _hanDakuten + _plain + _small;
  }

private:
  uint16_t _dakuten{}, _hanDakuten{}, _plain{}, _small{};
};

auto& operator<<(std::ostream& os, const KanaCount& k) {
  os << k.total() << " (Plain=" << k.plain() << ", Dakuten=" << k.dakuten()
     << ", HanDakuten=" << k.hanDakuten();
  if (k.small()) os << ", Small=" << k.small();
  return os << ')';
}

void printChartHeader(std::ostream& out, bool markdown) {
  out << (markdown ? "**Notes:**\n" : ">>> Notes:") << R"(
- Column names use some abbreviations: 'Roma' = Rōmaji, 'Hira' = Hiragana,
  'Kata' = Katakana, 'Uni' = Unicode, 'Hepb' = Hepburn, 'Kunr' = Kunrei
- 'Roma' is mainly 'Modern Hepburn', but can be 'Nihon Shiki' or 'Wāpuro'
- 'Hepb' and 'Kunr' are only populated when they would produce different output
  - Values in () means 'output-only' since inputting leads to a different Kana
- 'Roma Variants' are alternative key combinations that lead to the same Kana
- When populated, 'Roma', 'Hira' and 'Kata' columns are unique (no duplicates)
- Unicode values are only shown for 'monograph' (single-symbol) entries
- Some 'digraphs' may not be in any 'official words', but are supported by IMEs
- Chart output is sorted by Hiragana, so 'a, ka, sa, ta, na, ...' ordering
- Katakana 'dakuten w' (ヷ, ヸ, ヹ, ヺ) aren't supported (no conversions exist)
- Type values: P=Plain Kana, D=Dakuten (濁点), H=HanDakuten (半濁点), N=None
- Type 'N' includes:
  - Middle Dot/Interpunct (・): maps to Rōmaji '/' to match IME keyboard entry
  - Prolong Mark (ー): convert to/from macrons (ā, ī, ū, ē, ō)
  - Repeat symbols (ゝ, ゞ, ヽ, ヾ): only supported when 'target' is Rōmaji
)";
  if (markdown) out << '\n';
}

void printChartFooter(std::ostream& out, bool markdown, size_t romajiVariants,
    const KanaCount& monographs, const KanaCount& digraphs) {
  static constexpr uint16_t NoneTypeKana{4}, FooterWidth{10};
  out << '\n'
      << (markdown ? "**Totals:**\n" : ">>> Totals:") << std::setfill(' ')
      << std::right << '\n';
  const auto print{[&out, markdown](const String& s) -> std::ostream& {
    if (markdown) return out << "- **" << s << ":** ";
    return out << std::setw(FooterWidth) << s << ": " << std::setw(3);
  }};
  print("Monographs") << monographs << '\n';
  print("Digraphs") << digraphs << '\n';
  print("All Kana") << monographs.total() + digraphs.total()
                    << " (Monographs=" << monographs.total()
                    << ", Digraphs=" << digraphs.total()
                    << "), Rōmaji Variants=" << romajiVariants << '\n';
  // 'small' Kana are plain monographs, but are counted separately (digraphs
  // always consist of a full size Kana followed by a small Kana)
  const auto plain{monographs.small() + monographs.plain() + digraphs.plain()},
      dakuten{monographs.dakuten() + digraphs.dakuten()},
      hanDakuten{monographs.hanDakuten() + digraphs.hanDakuten()};
  const auto types{plain + dakuten + hanDakuten + NoneTypeKana};
  print("Types") << types << " (P=" << plain << ", D=" << dakuten
                 << ", H=" << hanDakuten << ", N=" << NoneTypeKana
                 << "), N types aren't included in 'All Kana'\n";
}

[[nodiscard]] String getHepburn(const Kana& i, const String& romaji) {
  auto& hepburn{i.getRomaji(ConvertFlags::Hepburn)};
  return romaji == hepburn ? emptyString()
                           : addBrackets(hepburn, BracketType::Round);
}

[[nodiscard]] String getKunrei(const Kana& i, const String& romaji) {
  auto& kunrei{i.getRomaji(ConvertFlags::Kunrei)};
  return romaji == kunrei    ? emptyString()
         : i.kunreiVariant() ? kunrei
                             : addBrackets(kunrei, BracketType::Round);
}

} // namespace

void KanaConvert::error(const String& msg) { throw DomainError(msg); }

KanaConvert::KanaConvert(const Args& args, std::ostream& out, std::istream* in)
    : _out(out), _in(in), _choice{out, in} {
  auto printKana{false}, printMarkdown{false};
  List strings;
  for (Args::Size i{1}; i < args.size(); ++i)
    if (String arg{args[i]}; arg == "--")
      while (++i < args.size()) strings.emplace_back(args[i]);
    else if (arg == "-?") {
      usage();
      return;
    } else if (arg == "-f") {
      if (++i >= args.size()) error("-f must be followed by a flag value");
      if (arg = args[i]; arg.size() != 1 || !flagArgs(arg[0]))
        error("illegal option for -f: " + arg);
    } else if (!processArg(arg, printKana, printMarkdown))
      strings.emplace_back(arg);

  if (!strings.empty()) {
    if (_interactive || printKana || printMarkdown)
      error("'string' args can't be combined with '-i', '-m' or '-p'");
    start(strings);
  } else if (printKana || printMarkdown)
    printKanaChart(printMarkdown);
  else {
    // when testing ('_in' is defined) or reading from a tty then require
    // '-i' if no string args are provided. If stdin is not a tty (like a
    // pipe) then '-i' isn't required, e.g.: 'echo hi | kanaConvert'
    if ((_in || isatty(fileno(stdin))) && !_interactive)
      error("provide one or more 'strings' or '-i' for interactive mode");
    start();
  }
}

bool KanaConvert::processArg(
    const String& arg, bool& printKana, bool& printMarkdown) {
  const auto setBool{[this, &printKana, &printMarkdown](bool& b) {
    // NOLINTNEXTLINE: NonNullParamChecker
    if (_interactive || _suppressNewLine || printKana || printMarkdown)
      error("can only specify one of -i, -m, -n, or -p");
    b = true;
  }};
  if (arg == "-i")
    setBool(_interactive);
  else if (arg == "-m")
    setBool(printMarkdown);
  else if (arg == "-n")
    setBool(_suppressNewLine);
  else if (arg == "-p")
    setBool(printKana);
  else if (arg.starts_with("-")) {
    if (!charTypeArgs(arg)) error("illegal option: " + arg);
  } else
    return false; // arg is not an 'option' so treat as a string to convert
  return true;    // arg was processed successfully
}

bool KanaConvert::charTypeArgs(const String& arg) {
  // this function is only called when 'arg' starts with '-' so don't need to
  // check that again (only need to check the length)
  if (arg.size() != 2) return false;
  switch (arg[1]) {
  case 'h': _converter.target(CharType::Hiragana); break;
  case 'k': _converter.target(CharType::Katakana); break;
  case 'r': _converter.target(CharType::Romaji); break;
  case 'H': _source = CharType::Hiragana; break;
  case 'K': _source = CharType::Katakana; break;
  case 'R': _source = CharType::Romaji; break;
  default: return false;
  }
  return true;
}

bool KanaConvert::flagArgs(char arg) {
  switch (arg) {
  case 'h': setFlag(ConvertFlags::Hepburn); break;
  case 'k': setFlag(ConvertFlags::Kunrei); break;
  case 'n': setFlag(ConvertFlags::NoProlongMark); break;
  case 'r': setFlag(ConvertFlags::RemoveSpaces); break;
  default: return false;
  }
  return true;
}

void KanaConvert::usage(bool showAllOptions) const {
  if (showAllOptions) {
    _out << R"(usage: kanaConvert -i
       kanaConvert [-n] string ...
       kanaConvert -m|-p|-?
  -i: interactive mode
  -n: suppress newline on output (for non-interactive mode)
  -m: print Kana chart in 'Markdown' format and exit
  -p: print Kana chart aligned for terminal output and exit
  -?: prints this usage message
  --: finish options, subsequent args are treated as strings to convert
  string ...: one or more strings to convert, no strings means read stdin
  
options for setting conversion source and target types as well as conversion
related flags can also be specified:
  -f opt: set 'opt' (can use multiple times to combine options). Options are:
    h: conform Rōmaji output more closely to 'Modern Hepburn' style
    k: conform Rōmaji output more closely to 'Kunrei Shiki' style
    n: no prolonged marks (repeat vowels instead of 'ー' for Hiragana output)
    r: remove spaces on output (only applies to Hiragana and Katakana output)
)";
  }
  _out << "  -h: set conversion output to Hiragana"
       << (showAllOptions ? " (default)" : "") << R"(
  -k: set conversion output to Katakana
  -r: set conversion output to Rōmaji
  -H: restrict conversion input to Hiragana
  -K: restrict conversion input to Katakana
  -R: restrict conversion input to Rōmaji
)";
}

void KanaConvert::start(const List& strings) {
  if (strings.empty())
    getInput();
  else {
    for (auto space{false}; auto& i : strings) {
      if (space)
        _out << (_converter.target() == CharType::Romaji ? " " : "　");
      else
        space = _converter.target() != CharType::Romaji &&
                !(_converter.flags() & ConvertFlags::RemoveSpaces);
      convert(i);
    }
    _out << '\n';
  }
}

void KanaConvert::getInput() {
  if (_interactive) printOptions();
  for (String line; std::getline(_in ? *_in : std::cin, line) && line != "q";) {
    if (_interactive && !processLine(line)) continue;
    convert(line);
    if (!_suppressNewLine) _out << '\n';
  }
}

void KanaConvert::printOptions() const {
  _out << ">>> current options: source="
       << (_source ? toString(*_source) : "any")
       << ", target=" << toString(_converter.target())
       << ", flags=" << _converter.flagString()
       << "\n>>> enter string (c=clear flags, f=set flag, q=quit, h=help, "
          "-k|-h|-r|-K|-H|-R):\n";
}

bool KanaConvert::processLine(const String& line) {
  if (line.empty()) return false;
  static const Choice::Choices flagChoices{{'h', "Hepburn"}, // GCOV_EXCL_LINE
      {'k', "Kunrei"}, {'n', "NoProlongMark"}, {'r', "RemoveSpaces"}};
  if (line == "c")
    _converter.flags(ConvertFlags::None);
  else if (line == "f")
    flagArgs(_choice.get(">>> enter flag option", flagChoices));
  else if (line == "h")
    usage(false);
  else if (line.starts_with("-")) {
    if (!charTypeArgs(line)) _out << "  illegal option: " << line << '\n';
  } else
    return true;
  printOptions();
  return false;
}

void KanaConvert::convert(const String& s) {
  _out << (_source ? _converter.convert(*_source, s) : _converter.convert(s));
}

void KanaConvert::setFlag(ConvertFlags value) {
  _converter.flags(_converter.flags() | value);
}

void KanaConvert::printKanaChart(bool markdown) const {
  printChartHeader(_out, markdown);
  size_t romajiVariants{};
  KanaCount monographs, digraphs;
  Table table{{"No.", "Type", "Roma", "Hira", "Kata", "HUni", "KUni", "Hepb",
                  "Kunr", "Roma Variants"},
      true};
  // Put a border before each 'group' of kana - use 'la', 'lya' and 'lwa' when
  // there are small letters that should be included, i.e., 'la' (ぁ) comes
  // right before 'a' (あ).
  const std::set<String> groups{
      "la", "ka", "sa", "ta", "na", "ha", "ma", "lya", "ra", "lwa"};
  for (auto& entry : Kana::getMap(CharType::Hiragana)) {
    auto& i{*entry.second};
    romajiVariants += i.romajiVariants().size();
    (i.isMonograph() ? monographs : digraphs).add(i);
    const String type{i.isDakuten() ? "D" : i.isHanDakuten() ? "H" : "P"};
    auto& romaji{i.romaji()};
    auto& h{i.hiragana()};
    auto& k{i.katakana()};
    const auto hepburn{getHepburn(i, romaji)}, kunrei{getKunrei(i, romaji)};
    String vars;
    for (auto& j : i.romajiVariants()) {
      if (!vars.empty()) vars += ", ";
      vars += j;
    }
    // only show unicode for monographs
    const auto getUni{[&i](auto& s) {
      return i.isMonograph() ? toUnicode(s) : emptyString();
    }};
    table.add({type, romaji, h, k, getUni(h), getUni(k), hepburn, kunrei, vars},
        groups.contains(romaji));
  }
  // special handling for middle dot, prolong mark and repeat marks
  const String slash{"/"}, middleDot{"・"};
  table.add({"N", slash, {}, middleDot, {}, toUnicode(middleDot)}, true);
  table.add({"N", {}, {}, Kana::ProlongMark, {}, toUnicode(Kana::ProlongMark)});
  for (auto& i : std::array{&Kana::RepeatPlain, &Kana::RepeatAccented}) {
    auto& h{i->hiragana()};
    auto& k{i->katakana()};
    table.add({"N", {}, h, k, toUnicode(h), toUnicode(k)});
  }
  markdown ? table.printMarkdown(_out) : table.print(_out);
  printChartFooter(_out, markdown, romajiVariants, monographs, digraphs);
}

} // namespace kanji_tools
