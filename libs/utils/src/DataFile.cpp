#include <kanji_tools/utils/DataFile.h>
#include <kanji_tools/utils/MBUtils.h>
#include <kanji_tools/utils/Utils.h>

#include <fstream>
#include <sstream>

namespace kanji_tools {

namespace fs = std::filesystem;

fs::path DataFile::getFile(const Path& dir, const Path& file) {
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

void DataFile::print(std::ostream& out, const StringList& l,
    const std::string& type, const std::string& group, bool isError) {
  if (!l.empty()) {
    out << (isError ? "ERROR ---" : ">>>") << " Found " << l.size() << ' '
        << type;
    if (!group.empty()) out << " in " << group;
    out << ':';
    for (auto& i : l) out << ' ' << i;
    out << '\n';
  }
}

void DataFile::clearUniqueCheckData() {
  UniqueNames.clear();
  for (auto i : OtherUniqueNames) i->clear();
}

DataFile::DataFile(const Path& p, FileType fileType)
    : DataFile{p, fileType, nullptr} {}

DataFile::DataFile(const Path& p)
    : DataFile{p, FileType::OnePerLine, nullptr} {}

DataFile::DataFile(const Path& fileIn, FileType fileType,
    StringSet* uniqueTypeNames, const std::string& name)
    : _name{name.empty() ? firstUpper(fileIn.stem().string()) : name} {
  auto file{fileIn};
  // try adding .txt if file isn't found
  if (!fs::is_regular_file(file) && !fileIn.has_extension())
    file += TextFileExtension;
  if (!fs::is_regular_file(file)) usage("can't open " + file.string());
  if (uniqueTypeNames) OtherUniqueNames.insert(uniqueTypeNames);
  auto lineNum{1};
  const auto error{[&lineNum, &file](const auto& s, bool pLine = true) {
    usage(s + (pLine ? " - line: " + std::to_string(lineNum) : EmptyString) +
          ", file: " + file.string());
  }};
  std::ifstream f{file};
  DataFile::StringList dups;
  for (std::string line; std::getline(f, line); ++lineNum) {
    std::stringstream ss{line};
    for (std::string token; std::getline(ss, token, ' ');) {
      if (fileType == FileType::OnePerLine && token != line)
        error("got multiple tokens");
      if (!isValidMBUtf8(token, true))
        error("invalid multi-byte token '" + token + "'");
      // check uniqueness within file
      if (_map.find(token) != _map.end())
        error("got duplicate token '" + token);
      // check uniqueness across files
      if (uniqueTypeNames) {
        if (const auto i{uniqueTypeNames->insert(token)}; !i.second) {
          dups.emplace_back(*i.first);
          continue;
        }
      } else if (!UniqueNames.insert(token).second)
        error("found globally non-unique entry '" + token + "'");
      _list.emplace_back(token);
      // 'value' starts at 1, i.e., the first kanji has 'frequency 1' (not 0)
      _map[token] = _list.size();
    }
  }
  if (!dups.empty()) {
    std::string msg{"found " + std::to_string(dups.size()) + " duplicates in " +
                    _name + ":"};
    for (const auto& i : dups) msg += ' ' + i;
    error(msg, false);
  }
}

bool DataFile::exists(const std::string& s) const {
  return _map.find(s) != _map.end();
}

size_t DataFile::getIndex(const std::string& name) const {
  const auto i{_map.find(name)};
  return i != _map.end() ? i->second : 0;
}

std::string DataFile::toString() const {
  std::string result;
  // reserve for efficiency - make a guess that each entry in the list is a 3
  // byte utf8 character
  result.reserve(_list.size() * 3);
  for (auto& i : _list) result += i;
  return result;
}

} // namespace kanji_tools
