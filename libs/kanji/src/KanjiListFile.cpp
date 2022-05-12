#include <kanji_tools/kanji/KanjiListFile.h>
#include <kanji_tools/utils/MBUtils.h>

#include <fstream>
#include <sstream>

namespace kanji_tools {

namespace fs = std::filesystem;

fs::path KanjiListFile::getFile(const Path& dir, const Path& file) {
  if (!fs::is_directory(dir)) usage(dir.string() + " is not a directory");
  Path p{dir / file};
  if (!fs::is_regular_file(p) && !file.has_extension()) p += TextFileExtension;
  if (!fs::exists(p)) {
    auto msg{dir.string() + " must contain '" + file.string()};
    usage(file.has_extension()
              ? msg + '\''
              : msg + "' (also tried '" + TextFileExtension + "' extension)");
  }
  if (!fs::is_regular_file(p)) usage(file.string() + " must be a regular file");
  return p;
}

void KanjiListFile::print(std::ostream& out, const StringList& l,
    const String& type, const String& group) {
  if (!l.empty()) {
    out << ">>> Found " << l.size() << ' ' << type;
    if (!group.empty()) out << " in " << group;
    out << ':';
    for (auto& i : l) out << ' ' << i;
    out << '\n';
  }
}

void KanjiListFile::clearUniqueCheckData() {
  UniqueNames.clear();
  for (auto i : OtherUniqueNames) i->clear();
}

KanjiListFile::KanjiListFile(const Path& p, FileType fileType)
    : KanjiListFile{p, fileType, {}} {}

KanjiListFile::KanjiListFile(const Path& fileIn, FileType fileType,
    StringSet* uniqueTypeNames, const String& name)
    : _name{name.empty() ? firstUpper(fileIn.stem().string()) : name} {
  auto file{fileIn};
  // try adding .txt if file isn't found
  if (!fs::is_regular_file(file) && !fileIn.has_extension())
    file += TextFileExtension;
  if (!fs::is_regular_file(file)) usage("can't open " + file.string());
  if (uniqueTypeNames) OtherUniqueNames.insert(uniqueTypeNames);
  load(file, fileType, uniqueTypeNames);
}

void KanjiListFile::load(
    const Path& file, FileType fileType, StringSet* uniqueTypeNames) {
  auto lineNum{1};
  const auto error{[&lineNum, &file](const auto& s, bool pLine = true) {
    usage(s + (pLine ? " - line: " + std::to_string(lineNum) : EmptyString) +
          ", file: " + file.string());
  }};
  std::ifstream f{file};
  KanjiListFile::StringList dups;
  for (String line; std::getline(f, line); ++lineNum) {
    std::stringstream ss{line};
    for (String token; std::getline(ss, token, ' ');)
      if (fileType == FileType::OnePerLine && token != line)
        error("got multiple tokens");
      else if (!validate(error, uniqueTypeNames, token))
        dups.emplace_back(token);
      else if (!addEntry(token))
        error("exceeded '" + std::to_string(MaxEntries) + "' entries", false);
  }
  if (!dups.empty()) {
    String msg{"found " + std::to_string(dups.size()) + " duplicates in " +
               _name + ":"};
    for (const auto& i : dups) msg += ' ' + i;
    error(msg, false);
  }
}

template<typename T>
bool KanjiListFile::validate(
    const T& error, StringSet* uniqueTypeNames, const String& token) {
  if (!isValidMBUtf8(token, true))
    error("invalid multi-byte token '" + token + "'");
  // check uniqueness within file
  if (_map.find(token) != _map.end()) error("got duplicate token '" + token);
  // check uniqueness across files
  if (uniqueTypeNames) return uniqueTypeNames->insert(token).second;
  if (!UniqueNames.insert(token).second)
    error("found globally non-unique entry '" + token + "'");
  return true;
}

bool KanjiListFile::addEntry(const String& token) {
  if (_list.size() == MaxEntries) return false;
  _list.emplace_back(token);
  // 'index' starts at 1, i.e., the first kanji has 'frequency 1' (not 0)
  _map.emplace(token, _list.size());
  return true;
}

bool KanjiListFile::exists(const String& s) const {
  return _map.find(s) != _map.end();
}

KanjiListFile::Index KanjiListFile::getIndex(const String& name) const {
  const auto i{_map.find(name)};
  return i != _map.end() ? i->second : Index{};
}

String KanjiListFile::toString() const {
  String result;
  // reserve for efficiency - make a guess that each entry in the list is a 3
  // byte utf8 character
  result.reserve(_list.size() * 3);
  for (auto& i : _list) result += i;
  return result;
}

} // namespace kanji_tools
