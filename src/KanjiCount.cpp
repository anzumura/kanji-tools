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

KanjiCount::KanjiCount(int argc, const char** argv) : KanjiData(argc, argv) {
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
      std::cout << "command line options:\n  -b file: show wide-character counts and full kanji breakdown for 'file'\n"
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
    std::cout << "Rank  [Kanji #] Freq, LV, Type (No.) == Highest Count File (if not found)\n";
    FileList::List missing;
    std::map<Types, int> types;
    for (const auto& i : frequency) {
      std::cout << std::left << std::setw(5) << ++rank << ' ' << i;
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
          std::cout << " == " << file;
        }
      }
      std::cout << '\n';
    }
    if (!types.empty()) {
      out() << "Types:\n";
      for (auto i : types)
        std::cout << "  " << i.first << ": " << i.second << '\n';
    }
    FileList::print(missing, "missing");
  }
  if (total)
    out() << std::right << std::setw(16) << name << ": " << std::setw(6) << total << ", unique: " << std::setw(4)
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
  out() << "Total Kanji+Kana: " << total << " (" << std::fixed << std::setprecision(1);
  for (int i = 0; i < IncludeInTotals; ++i)
    if (totals[i].first) {
      if (totals[i].second != totals[0].second) std::cout << ", ";
      std::cout << totals[i].second << ": " << totals[i].first * 100. / total << "%";
    }
  std::cout << ")\n";
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
    out() << name << ' ' << total << " (";
    for (const auto& i : counts) {
      std::cout << i.first << ' ' << i.second;
      total -= i.second;
      if (total) std::cout << ", ";
    }
    std::cout << ")\n";
  }
}

void KanjiCount::printStats() const {
  out() << "Loaded " << _map.size() << " Kanji (";
  for (const auto& i : _types) {
    if (i != *_types.begin()) std::cout << ' ';
    std::cout << i.first << ' ' << i.second.size();
  }
  std::cout << ")\n";
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
  out() << "Grade breakdown:\n";
  int all = 0;
  const auto& jouyou = _types.at(Types::Jouyou);
  for (auto i : AllGrades) {
    auto grade = [i](const auto& x) { return x->grade() == i; };
    auto gradeCount = std::count_if(jouyou.begin(), jouyou.end(), grade);
    if (gradeCount) {
      all += gradeCount;
      out() << "  Total for grade " << i << ": " << gradeCount;
      noFreq(
        std::count_if(jouyou.begin(), jouyou.end(), [&grade](const auto& x) { return grade(x) && !x->frequency(); }),
        true);
      std::cout << " (";
      for (auto level : AllLevels) {
        const auto gradeLevelCount = std::count_if(
          jouyou.begin(), jouyou.end(), [&grade, level](const auto& x) { return grade(x) && x->level() == level; });
        if (gradeLevelCount) {
          gradeCount -= gradeLevelCount;
          std::cout << level << ' ' << gradeLevelCount;
          if (gradeCount) std::cout << ", ";
        }
      }
      std::cout << ")\n";
    }
  }
  out() << "  Total for all grades: " << all << '\n';
}

void KanjiCount::printLevels() const {
  out() << "Level breakdown:\n";
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
      out() << "  Total for level " << level << ": " << levelTotal << " (";
      for (const auto& j : counts) {
        std::cout << j.first << ' ' << j.second;
        const auto& l = _types.at(j.first);
        noFreq(
          std::count_if(l.begin(), l.end(), [level](const auto& x) { return x->level() == level && !x->frequency(); }));
        levelTotal -= j.second;
        if (levelTotal) std::cout << ", ";
      }
      std::cout << ")\n";
    }
  }
  out() << "  Total for all levels: " << total << '\n';
}

void KanjiCount::printRadicals() const {
  out() << "Radical breakdown - total count for each name is followed by (Jouyou Jinmei Extra) counts:\n";
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
    std::cout << i.first << ':' << std::setfill(' ') << std::right << std::setw(4) << i.second.size() << " ("
              << std::left << std::setw(9) << counts << ':';
    jouyou += jo;
    jinmei += ji;
    extra += ex;
    Types oldType = i.second[0]->type();
    for (const auto& j : i.second) {
      if (j->type() != oldType) {
        std::cout << "、";
        oldType = j->type();
      }
      std::cout << ' ' << *j;
    }
    std::cout << '\n';
  }
  out() << "  Total for " << radicals.size() << " radicals: " << jouyou + jinmei + extra << " (Jouyou " << jouyou
        << " Jinmei " << jinmei << " Extra " << extra << ")\n";
  std::vector<Radical> missingRadicals;
  for (const auto& i : _radicals)
    if (radicals.find(i.second) == radicals.end()) missingRadicals.push_back(i.second);
  if (!missingRadicals.empty()) {
    out() << "  Found " << missingRadicals.size() << " radicals with no kanji:";
    for (const auto& i : missingRadicals)
      std::cout << ' ' << i;
    std::cout << '\n';
  }
}

} // namespace kanji
