#include <kanji_tools/quiz/JukugoData.h>
#include <kanji_tools/utils/MBChar.h>
#include <kanji_tools/utils/UnicodeBlock.h>

#include <fstream>
#include <sstream>

namespace kanji_tools {

namespace fs = std::filesystem;

JukugoData::JukugoData(const Data& data) {
  const std::string jukugoDir("jukugo/");
  auto f = [this, &jukugoDir, &data](const char* file, KanjiGrades grade) {
    int loaded = loadFile(DataFile::getFile(data.dataDir(), fs::path(jukugoDir + file + ".txt")), grade);
    if (data.debug()) data.log() << "Loaded " << loaded << " for Grade: " << grade << '\n';
  };
  if (data.debug()) data.log(true) << "Begin Loading Jukugo\n>>>\n";
  f("g1", KanjiGrades::G1);
  f("g2", KanjiGrades::G2);
  f("g3", KanjiGrades::G3);
  f("g4", KanjiGrades::G4);
  f("g5", KanjiGrades::G5);
  f("g6", KanjiGrades::G6);
  f("other", KanjiGrades::S);
  if (data.debug()) {
    data.log() << "Total Kanji with Jukugo: " << _kanjiToJukugo.size() << ", unique jukugo: " << _uniqueJukugo.size()
               << '\n';
    std::map<KanjiTypes, std::vector<std::string>> types;
    for (auto& i : _kanjiToJukugo)
      types[data.getType(i.first)].push_back(i.first);
    for (auto& i : types) {
      data.out() << std::right << std::setw(14) << i.first << ": " << i.second.size() << ' ';
      for (int j = 0; j < i.second.size() && j < 12; ++j)
        data.out() << (j == 0 ? '(' : ' ') << i.second[j];
      data.out() << ")\n";
    }
  }
}

int JukugoData::loadFile(const fs::path& file, KanjiGrades grade) {
  static const std::string stripPrefix("..."), openBracket("("), closeBracket(")");
  const int previouslyCreated = _uniqueJukugo.size();
  std::ifstream f(file);
  int lineNumber = 1;
  auto error = [&lineNumber, &file](const std::string& s) {
    Data::usage(s + " - line: " + std::to_string(lineNumber) + ", file: " + file.string());
  };
  // There are some duplicates in data files which makes sense for 'jukugo/other.txt' since it has a
  // line per kanji followed by jukugo, but there are also duplicate in the 'jukugo/g*.txt' files. For
  // example 一見 is in 'g1.txt' three times, once with reading いちげん and twice with reading いっけん.
  // Different readings are of course ok, but for now silently ignore duplicates with the same reading
  // in the same grade file (and report an error if a duplicate is found in a different file).
  auto create = [this, &error, grade](const std::string& name, const std::string& reading) {
    JukugoKey key{name, reading};
    auto i = _uniqueJukugo.find(key);
    if (i == _uniqueJukugo.end()) {
      MBChar n(name), r(reading);
      int count = 0;
      std::string x;
      while (n.next(x))
        if (isKanji(x)) ++count;
      if (count < 2) error("jukugo '" + name + "' must consist of two or more kanji");
      while (r.next(x))
        if (!isHiragana(x) && x != Kana::ProlongMark) error("jukugo '" + name + "' reading must be all hiragana");
      auto jukugo = std::make_shared<Jukugo>(name, reading, grade);
      n.reset();
      while (n.next(x))
        if (isKanji(x)) _kanjiToJukugo[x].push_back(jukugo);
      _uniqueJukugo.insert(std::make_pair(key, jukugo));
    } else if (i->second->grade() != grade)
      error("jukugo '" + name + "' found in more than one grade file");
  };
  for (std::string line; std::getline(f, line); ++lineNumber) {
    auto i = line.find(stripPrefix);
    if (i == std::string::npos) {
      // line has one entry with a space between the Jukugo and its bracketed reading (so g*.txt files)
      i = line.find(openBracket);
      if (i == std::string::npos) error("failed to find open bracket");
      if (i < 2 || line[i - 1] != ' ') error("open bracket should follow a space");
      auto j = line.find(closeBracket);
      if (j == std::string::npos) error("failed to find close bracket");
      if (j != line.length() - 1) error("close bracket should be the last character");
      create(line.substr(0, i - 1), line.substr(i + 1, j - i - 1));
    } else {
      // line has multiple space separated Jukugo entries for a given kanji (so other.txt file), i.e.:
      // X ... XA(reading) XB(reading) XC(reading)
      std::stringstream ss(line.substr(i + stripPrefix.length() + 1));
      while (std::getline(ss, line, ' ')) {
        i = line.find(openBracket);
        if (i == std::string::npos) error("failed to find open bracket");
        auto j = line.find(closeBracket);
        if (j == std::string::npos) error("failed to find close bracket");
        create(line.substr(0, i), line.substr(i + 1, j - i - 1));
      }
    }
  }
  return _uniqueJukugo.size() - previouslyCreated;
}

} // namespace kanji_tools