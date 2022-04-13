#include <kanji_tools/kana/KanaConvert.h>
#include <kanji_tools/kana/Table.h>
#include <kanji_tools/utils/Utils.h>

#include <cstdio>
#include <unistd.h>

#include <filesystem>

namespace kanji_tools {

namespace fs = std::filesystem;

void KanaConvert::error(const std::string& msg) {
  throw std::domain_error(msg);
}

KanaConvert::KanaConvert(Args args, std::istream& in, std::ostream& out)
    : _in(in), _out(out), _choice{out, &in},
      _program{args ? fs::path(args[0]).filename().string()
                    : std::string{"kanaConvert"}} {
  auto finishedOptions{false}, printKana{false}, printMarkdown{false};
  const auto setBool{[this, &printKana, &printMarkdown](bool& b) {
    if (_interactive || _suppressNewLine || printKana || printMarkdown)
      error("can only specify one of -i, -m, -n, or -p");
    b = true;
  }};
  for (Args::Size i{1}; i < args.size(); ++i) {
    std::string arg{args[i]};
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
    else if (arg == "-?") {
      usage();
      return;
    } else if (arg == "-f") {
      if (i + 1 < args.size()) {
        ++i;
        arg = args[i];
        if (arg.size() != 1 || !flagArgs(arg[0]))
          error("illegal option for -f: " + arg);
      } else
        error("-f must be followed by a flag value");
    } else if (arg.starts_with("-")) {
      if (!charTypeArgs(arg)) error("illegal option: " + arg);
    } else
      _strings.emplace_back(arg);
  }
  if (_strings.empty()) {
    if (isatty(fileno(stdin))) {
      if (printKana)
        printKanaChart();
      else if (printMarkdown)
        printKanaChart(true);
      else if (!_interactive)
        error("provide one or more 'strings' to convert or specify '-i' for "
              "interactive mode");
      else
        start();
    }
  } else if (_interactive)
    error("'-i' can't be combined with other 'string' arguments");
  else
    start();
}

bool KanaConvert::charTypeArgs(const std::string& arg) {
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

bool KanaConvert::flagArgs(char arg) {
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

void KanaConvert::usage(bool showAllOptions) const {
  if (showAllOptions)
    _out << "usage: " << _program
         << " [-h|-k|-r] [-H|-K|-R] [-f h|n|r] [-i|-m|-n|-p] [string ...]\n";
  _out << "  -h: set conversion output to Hiragana"
       << (showAllOptions ? " (default)" : "") << R"(
  -k: set conversion output to Katakana
  -r: set conversion output to Rōmaji
  -H: restrict conversion input to Hiragana
  -K: restrict conversion input to Katakana
  -R: restrict conversion input to Rōmaji
)";
  if (showAllOptions) {
    _out << R"(  -?: prints this usage message
  -f option: set 'option' (-f can be used multiple times to combine options). Valid options are:
      h: conform Rōmaji output more closely to 'Modern Hepburn' style
      k: conform Rōmaji output more closely to 'Kunrei Shiki' style
      n: no prolonged sound marks on Hiragana output, i.e., vowels are repeated instead of 'ー'
      r: remove spaces on output (only applies to Hiragana and Katakana output)
  -i: interactive mode
  -m: print Kana chart in 'Markdown' format and exit
  -n: suppress newline on output (for non-interactive mode)
  -p: print Kana chart aligned for terminal output and exit
  --: finish parsing options, all further arguments will be treated as input files
  [string ...]: provide one or more strings to convert, no strings means process standard input
)";
  }
}

void KanaConvert::start() {
  if (_strings.empty())
    getInput();
  else {
    for (auto space{false}; const auto& i : _strings) {
      if (space)
        _out << (_converter.target() == CharType::Romaji ? " " : "　");
      else
        space = _converter.target() != CharType::Romaji &&
                !(_converter.flags() & ConvertFlags::RemoveSpaces);
      processOneLine(i);
    }
    _out << '\n';
  }
}

void KanaConvert::getInput() {
  static const Choice::Choices FlagChoices{{'h', "Hepburn"}, {'k', "Kunrei"},
      {'n', "NoProlongMark"}, {'r', "RemoveSpaces"}};
  auto outputCurrentOptions{true};
  do {
    if (_interactive && outputCurrentOptions) {
      _out << ">>> Current options: source="
           << (_source ? toString(*_source) : "any")
           << ", target=" << toString(_converter.target())
           << ", flags=" << _converter.flagString()
           << "\n>>> Enter string or 'c'=clear flags, 'f'=set flag, "
              "'q'=quit, 'h'=help or -k|-h|-r|-K|-H|-R:\n";
      outputCurrentOptions = false;
    }
    if (std::string line; std::getline(_in, line) && line != "q") {
      if (_interactive) {
        if (line.empty()) continue;
        if (line == "c" || line == "f" || line == "h" ||
            line.starts_with("-")) {
          if (line == "c")
            _converter.flags(ConvertFlags::None);
          else if (line == "f")
            flagArgs(_choice.get(">>> Enter flag option", FlagChoices));
          else if (line == "h") {
            usage(false);
            line = "q";
          } else if (line.starts_with("-")) {
            if (!charTypeArgs(line))
              _out << "  illegal option: " << line << '\n';
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

void KanaConvert::processOneLine(const std::string& s) {
  _out << (_source ? _converter.convert(*_source, s) : _converter.convert(s));
}

void KanaConvert::setFlag(ConvertFlags value) {
  _converter.flags(_converter.flags() | value);
}

void KanaConvert::printKanaChart(bool markdown) const {
  _out << (markdown ? "## **Kana Conversion Chart**\n### **Notes:**"
                    : ">>> Notes:");
  _out << R"(
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
  u_int16_t hanDakutenMonographs{}, small{}, plainMonographs{},
      dakutenMonographs{}, plainDigraphs{}, hanDakutenDigraphs{},
      dakutenDigraphs{}, romajiVariants{};
  constexpr u_int16_t noneTypeKana{4};
  Table table{{"No.", "Type", "Roma", "Hira", "Kata", "HUni", "KUni", "Hepb",
                  "Kunr", "Roma Variants"},
      true};
  // Put a border before each 'group' of kana - use 'la', 'lya' and 'lwa' when
  // there are small letters that should be included, i.e., 'la' (ぁ) comes
  // right before 'a' (あ).
  const std::set<std::string> groups{
      "la", "ka", "sa", "ta", "na", "ha", "ma", "lya", "ra", "lwa"};
  for (auto& entry : Kana::getMap(CharType::Hiragana)) {
    auto& i{*entry.second};
    romajiVariants += static_cast<u_int16_t>(i.romajiVariants().size());
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
    auto hepb{i.getRomaji(ConvertFlags::Hepburn)},
        kunr{i.getRomaji(ConvertFlags::Kunrei)};
    hepb = romaji == hepb ? EmptyString : ('(' + hepb + ')');
    kunr = romaji == kunr      ? EmptyString
           : i.kunreiVariant() ? kunr
                               : ('(' + kunr + ')');
    std::string vars;
    for (auto j : i.romajiVariants()) {
      if (!vars.empty()) vars += ", ";
      vars += j;
    }
    // only show unicode for monographs
    const auto uni{
        [&i](auto& s) { return i.isMonograph() ? toUnicode(s) : EmptyString; }};
    table.add({type, romaji, h, k, uni(h), uni(k), hepb, kunr, vars},
        groups.contains(romaji));
  }
  // special handling for middle dot, prolong symbol and repeat symbols
  const std::string slash{"/"}, middleDot{"・"};
  table.add({"N", slash, {}, middleDot, {}, toUnicode(middleDot)}, true);
  table.add({"N", {}, {}, Kana::ProlongMark, {}, toUnicode(Kana::ProlongMark)});
  for (auto& i : std::array{&Kana::RepeatPlain, &Kana::RepeatAccented}) {
    auto& h{i->hiragana()};
    auto& k{i->katakana()};
    table.add({"N", {}, h, k, toUnicode(h), toUnicode(k)});
  }
  markdown ? table.printMarkdown(_out) : table.print(_out);
  const auto monographs{
      small + plainMonographs + dakutenMonographs + hanDakutenMonographs},
      digraphs{plainDigraphs + dakutenDigraphs + hanDakutenDigraphs},
      plain{small + plainMonographs + plainDigraphs},
      dakuten{dakutenMonographs + dakutenDigraphs},
      hanDakuten{hanDakutenMonographs + hanDakutenDigraphs};
  const auto types{plain + dakuten + hanDakuten + noneTypeKana};
  const auto out{[this, markdown](const std::string& s) -> std::ostream& {
    if (markdown)
      _out << "- **";
    else
      _out << std::setw(10);
    return _out << s << (markdown ? ":** " : ": ") << std::setw(3);
  }};
  _out << '\n'
       << (markdown ? "### **Totals:**" : ">>> Totals:") << std::setfill(' ')
       << std::right << '\n';
  out("Monograph") << monographs << " (Plain=" << plainMonographs
                   << ", Dakuten=" << dakutenMonographs
                   << ", HanDakuten=" << hanDakutenMonographs
                   << ", Small=" << small << ")\n";
  out("Digraphs") << digraphs << " (Plain=" << plainDigraphs
                  << ", Dakuten=" << dakutenDigraphs
                  << ", HanDakuten=" << hanDakutenDigraphs << ")\n";
  out("All Kana") << monographs + digraphs << " (Monographs=" << monographs
                  << ", Digraphs=" << digraphs
                  << "), Rōmaji Variants=" << romajiVariants << ")\n";
  out("Types") << types << " (P=" << plain << ", D=" << dakuten
               << ", H=" << hanDakuten << ", N=" << noneTypeKana
               << "), N types are not included in 'All Kana'\n";
}

} // namespace kanji_tools
