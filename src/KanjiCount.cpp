#include <kanji/Kanji.h>
#include <kanji/KanjiCount.h>
#include <kanji/MBChar.h>

#include <numeric>

namespace kanji {

namespace fs = std::filesystem;

namespace {

std::ostream& operator<<(std::ostream& os, const KanjiCount::Count& c) {
  os << '[' << c.name << ' ' << std::right << std::setw(4) << c.count << ']';
  if (c.entry.has_value())
    os << std::setw(5) << (**c.entry).frequency() << ", "
       << ((**c.entry).hasLevel() ? toString((**c.entry).level()) : std::string("--")) << ", " << (**c.entry).type()
       << " (" << (**c.entry).number() << ')';
  return os;
}

// helper function for printing 'no-frequency' counts
void noFreq(int f, bool brackets = false) {
  if (f) {
    if (brackets)
      std::cout << " (";
    else
      std::cout << ' ';
    std::cout << "nf " << f;
    if (brackets) std::cout << ')';
  }
}

} // namespace

KanjiCount::KanjiCount(int argc, const char** argv, std::ostream& out, std::ostream& err)
  : KanjiData(argc, argv, out, err) {
  if (_debug) {
    printStats();
    printGrades();
    printLevels();
    printRadicals();
  } else if (argc == 2)
    usage("please specify at least one option or '-h' for help");
  for (int i = _debug ? 3 : 2; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-b") {
      if (++i == argc) usage("-b must be followed by a file or directory name");
      countKanji(argv[i], true);
    } else if (arg == "-c") {
      if (++i == argc) usage("-c must be followed by a file or directory name");
      countKanji(argv[i]);
    } else if (arg == "-h") {
      _out << "command line options:\n  -b file: show wide-character counts and full kanji breakdown for 'file'\n"
           << "  -c file: show wide-character counts for 'file'\n"
           << "  -h: show help message for command-line options\n";
      return;
    } else
      usage("unrecognized arg: " + arg);
  }
}

int KanjiCount::Count::getFrequency() const {
  return entry.has_value() ? (**entry).frequencyOrDefault(MaxFrequency) : MaxFrequency + 1;
}

template<typename Pred>
int KanjiCount::processCount(const fs::path& top, const Pred& pred, const std::string& name, bool showBreakdown) const {
  // Furigana in a .txt file is usually a Kanji followed by one or more Hiragana characters inside
  // wide brackets. For now use a 'regex' that matches one Kanji followed by bracketed Hiragana (and
  // replace it with just the Kanji match). This should catch most reasonable examples.
  static const MBCharCount::OptRegex Furigana(std::wstring(L"([") + KanjiRange + L"]{1})（[" + HiraganaRange + L"]+）");
  const bool isKanji = name == "Kanji";
  const bool isUnrecognized = name == "Unrecognized";
  // remove furigana when processing Hiragana or MB-Letter to remove the effect on counts, i.e., furigana
  // in .txt files will artificially inflate Hiragana count (and MB-Letter because of the wide brackets)
  const bool removeFurigana = name == "Hiragana" || name == "MB-Letter";
  MBCharCountIf count(pred, removeFurigana ? Furigana : std::nullopt, "$1");
  count.addFile(top, isKanji || isUnrecognized);
  auto& m = count.map();
  std::set<Count> frequency;
  int total = 0, rank = 0;
  for (const auto& i : m) {
    total += i.second;
    frequency.emplace(i.second, i.first, isKanji ? findKanji(i.first) : std::nullopt);
  }
  if (total && (isUnrecognized || isKanji && showBreakdown)) {
    _out << "Rank  [Kanji #] Freq, LV, Type (No.) == Highest Count File (if not found)\n";
    FileList::List missing;
    std::map<Types, int> types;
    for (const auto& i : frequency) {
      _out << std::left << std::setw(5) << ++rank << ' ' << i;
      if (i.entry.has_value())
        types[(**i.entry).type()]++;
      else {
        missing.push_back(i.name);
        auto tags = count.tags(i.name);
        if (tags != nullptr) {
          int maxCount = 0;
          std::string file;
          for (const auto& j : *tags)
            if (j.second > maxCount) {
              maxCount = j.second;
              file = j.first;
            }
          _out << " == " << file;
        }
      }
      _out << '\n';
    }
    if (!types.empty()) {
      log() << "Types:\n";
      for (auto i : types)
        _out << "  " << i.first << ": " << i.second << '\n';
    }
    FileList::print(missing, "missing");
  }
  if (total)
    log() << std::right << std::setw(16) << name << ": " << std::setw(6) << total << ", unique: " << std::setw(4)
          << frequency.size() << " (directories: " << count.directories() << ", files: " << count.files() << ")\n";
  return total;
}

void KanjiCount::countKanji(const fs::path& top, bool showBreakdown) const {
  static const int IncludeInTotals = 3; // only include Kanji and full-width kana in total and percents
  auto f = [this, &top, showBreakdown](const auto& x, const auto& y) {
    return std::make_pair(this->processCount(top, x, y, showBreakdown), y);
  };
  std::array totals{f([](const auto& x) { return isKanji(x); }, "Kanji"),
                    f([](const auto& x) { return isHiragana(x); }, "Hiragana"),
                    f([](const auto& x) { return isKatakana(x); }, "Katakana"),
                    f([](const auto& x) { return isWidePunctuation(x, false); }, "MB-Punctuation"),
                    f([](const auto& x) { return isWideLetter(x); }, "MB-Letter"),
                    f([](const auto& x) { return !isRecognizedWide(x); }, "Unrecognized")};
  int total = 0;
  for (int i = 0; i < IncludeInTotals; ++i)
    total += totals[i].first;
  log() << "Total Kanji+Kana: " << total << " (" << std::fixed << std::setprecision(1);
  for (int i = 0; i < IncludeInTotals; ++i)
    if (totals[i].first) {
      if (totals[i].second != totals[0].second) std::cout << ", ";
      _out << totals[i].second << ": " << totals[i].first * 100. / total << "%";
    }
  _out << ")\n";
}

// Print functions called when -debug is specified

template<typename T> void KanjiCount::printCount(const std::string& name, T pred) const {
  std::vector<std::pair<Types, int>> counts;
  int total = 0;
  for (const auto& l : _types) {
    const int count = std::count_if(l.second.begin(), l.second.end(), pred);
    if (count) {
      counts.emplace_back(l.first, count);
      total += count;
    }
  }
  if (total) {
    log() << name << ' ' << total << " (";
    for (const auto& i : counts) {
      _out << i.first << ' ' << i.second;
      total -= i.second;
      if (total) _out << ", ";
    }
    _out << ")\n";
  }
}

void KanjiCount::printStats() const {
  log() << "Loaded " << _map.size() << " Kanji (";
  for (const auto& i : _types) {
    if (i != *_types.begin()) _out << ' ';
    _out << i.first << ' ' << i.second.size();
  }
  _out << ")\n";
  printCount("  Has JLPT level", [](const auto& x) { return x->hasLevel(); });
  printCount("  Has frequency and not in Jouyou or JLPT",
             [](const auto& x) { return x->frequency() && x->type() != Types::Jouyou && !x->hasLevel(); });
  printCount("  Jinmei with no frequency and not JLPT",
             [](const auto& x) { return x->type() == Types::Jinmei && !x->frequency() && !x->hasLevel(); });
  printCount("  NF (no-frequency)", [](const auto& x) { return !x->frequency(); });
  printCount("  Has Strokes", [](const auto& x) { return x->strokes() != 0; });
  printCount("Old Forms", [](const auto& x) { return x->oldName().has_value(); });
  // some old kanjis have a non-zero frequency
  printCount("  Old Has Frequency", [this](const auto& x) { return x->oldFrequency(*this) != 0; });
  // some old kanjis have stroke counts
  printCount("  Old Has Strokes", [this](const auto& x) { return x->oldStrokes(*this) != 0; });
  // no old kanjis should have a JLPT level, i.e.: they all should have Level 'None'
  printCount("  Old Has Level", [this](const auto& x) { return x->oldLevel(*this) != Levels::None; });
  // old kanjis should only have types of LinkedJinmei, Other or None
  for (auto i : AllTypes)
    printCount(std::string("  Old is type ") + toString(i),
               [this, i](const auto& x) { return x->oldName().has_value() && x->oldType(*this) == i; });
}

void KanjiCount::printGrades() const {
  log() << "Grade breakdown:\n";
  int all = 0;
  const auto& jouyou = _types.at(Types::Jouyou);
  for (auto i : AllGrades) {
    auto grade = [i](const auto& x) { return x->grade() == i; };
    auto gradeCount = std::count_if(jouyou.begin(), jouyou.end(), grade);
    if (gradeCount) {
      all += gradeCount;
      log() << "  Total for grade " << i << ": " << gradeCount;
      noFreq(
        std::count_if(jouyou.begin(), jouyou.end(), [&grade](const auto& x) { return grade(x) && !x->frequency(); }),
        true);
      _out << " (";
      for (auto level : AllLevels) {
        const auto gradeLevelCount = std::count_if(
          jouyou.begin(), jouyou.end(), [&grade, level](const auto& x) { return grade(x) && x->level() == level; });
        if (gradeLevelCount) {
          gradeCount -= gradeLevelCount;
          _out << level << ' ' << gradeLevelCount;
          if (gradeCount) _out << ", ";
        }
      }
      _out << ")\n";
    }
  }
  log() << "  Total for all grades: " << all << '\n';
}

void KanjiCount::printLevels() const {
  log() << "Level breakdown:\n";
  int total = 0;
  for (auto level : AllLevels) {
    std::vector<std::pair<Types, int>> counts;
    int levelTotal = 0;
    for (const auto& l : _types) {
      int count =
        std::count_if(l.second.begin(), l.second.end(), [level](const auto& x) { return x->level() == level; });
      if (count) {
        counts.emplace_back(l.first, count);
        levelTotal += count;
      }
    }
    if (levelTotal) {
      total += levelTotal;
      log() << "  Total for level " << level << ": " << levelTotal << " (";
      for (const auto& j : counts) {
        _out << j.first << ' ' << j.second;
        const auto& l = _types.at(j.first);
        noFreq(
          std::count_if(l.begin(), l.end(), [level](const auto& x) { return x->level() == level && !x->frequency(); }));
        levelTotal -= j.second;
        if (levelTotal) std::cout << ", ";
      }
      _out << ")\n";
    }
  }
  log() << "  Total for all levels: " << total << '\n';
}

void KanjiCount::printRadicals() const {
  log() << "Radical breakdown - total count for each name is followed by (Jouyou Jinmei Extra) counts:\n";
  std::map<Radical, Data::List> radicals;
  for (const auto& i : _types) {
    if (hasRadical(i.first)) {
      Data::List sorted(i.second);
      std::sort(sorted.begin(), sorted.end(), [](const auto& x, const auto& y) { return x->strokes() - y->strokes(); });
      for (const auto& j : sorted)
        radicals[static_cast<const FileListKanji&>(*j).radical()].push_back(j);
    }
  }
  int jouyou = 0, jinmei = 0, extra = 0;
  for (const auto& i : radicals) {
    int jo = 0, ji = 0, ex = 0;
    for (const auto& j : i.second)
      switch (j->type()) {
      case Types::Jouyou: ++jo; break;
      case Types::Jinmei: ++ji; break;
      default: ++ex; break;
      }
    auto counts = std::to_string(jo) + ' ' + std::to_string(ji) + ' ' + std::to_string(ex) + ')';
    _out << i.first << ':' << std::setfill(' ') << std::right << std::setw(4) << i.second.size() << " (" << std::left
         << std::setw(9) << counts << ':';
    jouyou += jo;
    jinmei += ji;
    extra += ex;
    Types oldType = i.second[0]->type();
    for (const auto& j : i.second) {
      if (j->type() != oldType) {
        _out << "、";
        oldType = j->type();
      }
      _out << ' ' << *j;
    }
    _out << '\n';
  }
  log() << "  Total for " << radicals.size() << " radicals: " << jouyou + jinmei + extra << " (Jouyou " << jouyou
        << " Jinmei " << jinmei << " Extra " << extra << ")\n";
  std::vector<Radical> missingRadicals;
  for (const auto& i : _radicals)
    if (radicals.find(i.second) == radicals.end()) missingRadicals.push_back(i.second);
  if (!missingRadicals.empty()) {
    log() << "  Found " << missingRadicals.size() << " radicals with no kanji:";
    for (const auto& i : missingRadicals)
      _out << ' ' << i;
    _out << '\n';
  }
}

} // namespace kanji
