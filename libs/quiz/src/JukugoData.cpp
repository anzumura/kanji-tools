#include <kanji_tools/kana/MBChar.h>
#include <kanji_tools/quiz/JukugoData.h>
#include <kanji_tools/utils/UnicodeBlock.h>

#include <fstream>
#include <sstream>

namespace kanji_tools {

namespace {

constexpr auto JukugoSetW{14}, JukugoMaxExamples{12};

} // namespace

namespace fs = std::filesystem;

JukugoData::JukugoData(const KanjiDataPtr& data, const KanjiData::Path* dir) {
  const auto jukugoDir{dir ? *dir : data->dataDir() / "jukugo"};
  const auto f{[this, &jukugoDir, data](const char* file, KanjiGrades grade) {
    const auto loaded{loadFile(KanjiListFile::getFile(jukugoDir, file), grade)};
    if (data->debug())
      data->log() << "Loaded " << loaded << " for Grade: " << grade << '\n';
  }};
  if (data->debug()) data->log(true) << "Begin Loading Jukugo\n>>>\n";
  f("g1", KanjiGrades::G1);
  f("g2", KanjiGrades::G2);
  f("g3", KanjiGrades::G3);
  f("g4", KanjiGrades::G4);
  f("g5", KanjiGrades::G5);
  f("g6", KanjiGrades::G6);
  f("other", KanjiGrades::S);
  if (data->debug()) {
    data->log() << "Total Kanji with Jukugo: " << _kanjiToJukugo.size()
                << ", unique jukugo: " << _uniqueJukugo.size() << '\n';
    std::map<KanjiTypes, std::vector<String>> types;
    for (auto& i : _kanjiToJukugo)
      types[data->getType(i.first)].emplace_back(i.first);
    for (auto& i : types) {
      data->out() << std::right << std::setw(JukugoSetW) << i.first << ": "
                  << i.second.size() << ' ';
      for (size_t j{}; j < i.second.size() && j < JukugoMaxExamples; ++j)
        data->out() << (j == 0 ? '(' : ' ') << i.second[j];
      data->out() << ")\n";
    }
  }
}

const JukugoData::List& JukugoData::find(const String& kanji) const {
  const auto i{_kanjiToJukugo.find(kanji)};
  return i != _kanjiToJukugo.end() ? i->second : EmptyList;
}

size_t JukugoData::loadFile(const fs::path& file, KanjiGrades grade) {
  const auto previouslyCreated{_uniqueJukugo.size()};
  std::ifstream f{file};
  auto lineNum{1};
  try {
    if (const bool onePerLine{grade != KanjiGrades::S}; onePerLine)
      // line has one entry with a space between the Jukugo and its bracketed
      // reading (so g*.txt files)
      for (String line; std::getline(f, line); ++lineNum)
        createJukugo(line, grade, onePerLine);
    else
      // line has multiple space separated Jukugo entries for a given kanji
      // (so other.txt file), i.e.: X ... XA(reading) XB(reading) XC(reading)
      for (String line; std::getline(f, line); ++lineNum) {
        static const String Dots{"..."};
        const auto i{line.find(Dots)};
        if (i == String::npos) error("line is missing '" + Dots + "'");
        for (std::stringstream ss{line.substr(i + Dots.size() + 1)};
             std::getline(ss, line, ' ');)
          createJukugo(line, grade, onePerLine);
      }
  } catch (const std::exception& e) {
    error(String{e.what()} + " - line: " + std::to_string(lineNum) +
          ", file: " + file.filename().string());
  }
  return _uniqueJukugo.size() - previouslyCreated;
}

size_t JukugoData::findOpenBracket(const String& line, bool onePerLine) {
  const auto i{line.find('(')};
  if (i == String::npos) error("failed to find open bracket");
  if (onePerLine && (i < 2 || line[i - 1] != ' '))
    error("open bracket should follow a space");
  return i;
}

size_t JukugoData::findCloseBracket(const String& line, bool onePerLine) {
  const auto i{line.find(')')};
  if (i == String::npos) error("failed to find close bracket");
  if (onePerLine && i != line.size() - 1)
    error("close bracket should be the last character");
  return i;
}

void JukugoData::error(const String& msg) { throw std::domain_error(msg); }

void JukugoData::createJukugo(
    const String& line, KanjiGrades grade, bool onePerLine) {
  const auto open{findOpenBracket(line, onePerLine)};
  const auto close{findCloseBracket(line, onePerLine)};
  const String name{line.substr(0, onePerLine ? open - 1 : open)},
      reading{line.substr(open + 1, close - open - 1)};
  // There are some duplicates in data files which makes sense for
  // 'jukugo/other.txt' since it has a line per kanji followed by jukugo, but
  // there are also duplicate in the 'jukugo/g*.txt' files. For example 一見 is
  // in 'g1.txt' three times, once with reading いちげん and twice with reading
  // いっけん. Different readings are of course ok, but for now silently ignore
  // duplicates with the same reading in the same grade file (and report an
  // error if a duplicate is found in a different file).
  JukugoKey key{name, reading};
  if (const auto i{_uniqueJukugo.find(key)}; i == _uniqueJukugo.end()) {
    const auto jukugo{std::make_shared<Jukugo>(name, reading, grade)};
    MBChar nameChars{name};
    for (String c; nameChars.next(c);)
      if (isKanji(c)) _kanjiToJukugo[c].emplace_back(jukugo);
    _uniqueJukugo.emplace(key, jukugo);
  } else if (i->second->grade() != grade)
    throw std::domain_error(
        "jukugo '" + name + "' found in more than one file");
}

} // namespace kanji_tools
