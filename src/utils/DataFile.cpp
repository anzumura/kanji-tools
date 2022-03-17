#include <kanji_tools/utils/DataFile.h>
#include <kanji_tools/utils/MBUtils.h>

#include <fstream>
#include <sstream>

namespace kanji_tools {

namespace fs = std::filesystem;

fs::path DataFile::getFile(const fs::path& dir, const fs::path& file) {
  if (!fs::is_directory(dir)) usage(dir.string() + " is not a directory");
  fs::path p(dir / file);
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

void DataFile::print(const List& l, const std::string& type,
    const std::string& group, bool isError, std::ostream& out) {
  if (!l.empty()) {
    out << (isError ? "ERROR ---" : ">>>") << " Found " << l.size() << ' '
        << type;
    if (!group.empty()) out << " in " << group;
    out << ':';
    for (auto& i : l) out << ' ' << i;
    out << '\n';
  }
}

DataFile::DataFile(const fs::path& fileIn, FileType fileType,
    bool createNewUniqueFile, Set* uniqueTypeNames, const std::string& name)
    : _name(name.empty() ? capitalize(fileIn.stem().string()) : name) {
  auto file{fileIn};
  // try adding .txt if file isn't found
  if (!fs::is_regular_file(file) && !fileIn.has_extension())
    file += TextFileExtension;
  if (!fs::is_regular_file(file)) usage("can't open " + file.string());
  if (uniqueTypeNames) OtherUniqueNames.insert(uniqueTypeNames);
  auto lineNum{1};
  const auto error{[&lineNum, &file](const auto& s, bool printLine = true) {
    usage(s + (printLine ? " - line: " + std::to_string(lineNum) : "") +
          ", file: " + file.string());
  }};
  std::ifstream f{file};
  DataFile::List good, dups;
  for (std::string line; std::getline(f, line); ++lineNum) {
    std::stringstream ss{line};
    for (std::string token; std::getline(ss, token, ' ');) {
      if (fileType == FileType::OnePerLine) {
        if (token != line) error("got multiple tokens");
      } else if (token.empty() || token == "　")
        continue; // skip empty tokens and 'wide spaces'
      if (!isValidMBUtf8(token, true))
        error("invalid multi-byte token '" + token + "'");
      // check uniqueness with file
      if (_map.find(token) != _map.end())
        error("got duplicate token '" + token);
      // check uniqueness across files
      if (uniqueTypeNames) {
        const auto i{uniqueTypeNames->insert(token)};
        if (!i.second) {
          dups.emplace_back(*i.first);
          continue;
        }
        good.emplace_back(*i.first);
      } else if (!UniqueNames.insert(token).second)
        error("found globally non-unique entry '" + token + "'");
      _list.push_back(token);
      // 'value' starts at 1, i.e., the first kanji has 'frequency 1' (not 0)
      _map[token] = _list.size();
    }
  }
  if (!dups.empty()) {
    if (good.empty())
      error("found " + std::to_string(dups.size()) + " duplicates in " + _name,
          false);
    else {
      std::cerr << ">>> found " << dups.size() << " duplicates in " << _name
                << ":";
      for (const auto& i : dups) std::cerr << ' ' << i;
      if (createNewUniqueFile) {
        fs::path newFile{file};
        newFile.replace_extension(fs::path("new"));
        std::cerr << "\n>>> saving " << good.size()
                  << " unique entries to: " << newFile.string() << '\n';
        std::ofstream of(newFile);
        for (const auto& i : good) of << i << '\n';
      }
    }
  }
}

} // namespace kanji_tools
