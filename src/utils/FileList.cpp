#include <kanji_tools/utils/FileList.h>
#include <kanji_tools/utils/MBChar.h>

#include <fstream>
#include <sstream>

namespace kanji_tools {

namespace fs = std::filesystem;

const char* toString(Levels x) {
  switch (x) {
  case Levels::N5: return "N5";
  case Levels::N4: return "N4";
  case Levels::N3: return "N3";
  case Levels::N2: return "N2";
  case Levels::N1: return "N1";
  default: return "None";
  }
}

const char* toString(Kyus x) {
  switch (x) {
  case Kyus::K10: return "K10";
  case Kyus::K9: return "K9";
  case Kyus::K8: return "K8";
  case Kyus::K7: return "K7";
  case Kyus::K6: return "K6";
  case Kyus::K5: return "K5";
  case Kyus::K4: return "K4";
  case Kyus::K3: return "K3";
  case Kyus::KJ2: return "KJ2";
  case Kyus::K2: return "K2";
  case Kyus::KJ1: return "KJ1";
  case Kyus::K1: return "K1";
  default: return "None";
  }
}

fs::path FileList::getFile(const fs::path& dir, const fs::path& file) {
  fs::path p(dir / file);
  if (!fs::exists(p)) usage(dir.string() + " must contain " + file.string());
  if (!fs::is_regular_file(p)) usage(file.string() + " must be a regular file");
  return p;
}

void FileList::print(const List& l, const std::string& type, const std::string& group, bool isError) {
  if (!l.empty()) {
    std::cout << (isError ? "ERROR ---" : ">>>") << " Found " << l.size() << ' ' << type;
    if (!group.empty()) std::cout << " in " << group;
    std::cout << ':';
    for (const auto& i : l)
      std::cout << ' ' << i;
    std::cout << '\n';
  }
}

FileList::FileList(const fs::path& file, FileType fileType, bool createNewUniqueFile, Set* uniqueTypeNames,
                   const std::string& name)
  : _name(name.empty() ? capitalize(file.stem().string()) : name) {
  if (!fs::is_regular_file(file)) usage("can't open " + file.string());
  if (uniqueTypeNames) OtherUniqueNames.insert(uniqueTypeNames);
  int lineNumber = 1;
  auto error = [&lineNumber, &file](const std::string& s, bool printLine = true) {
    usage(s + (printLine ? " - line: " + std::to_string(lineNumber) : "") + ", file: " + file.string());
  };
  std::ifstream f(file);
  FileList::List good, dups;
  for (std::string line; std::getline(f, line); ++lineNumber) {
    std::stringstream ss(line);
    for (std::string token; std::getline(ss, token, ' ');) {
      if (fileType == FileType::OnePerLine) {
        if (token != line) error("got multiple tokens");
      } else if (token.empty() || token == "　")
        continue; // skip empty tokens and 'wide spaces' when processing multiple entries per line
      if (!MBChar::isValid(token)) error("invalid multi-byte token '" + token + "'");
      // check uniqueness with file
      if (_map.find(token) != _map.end()) error("got duplicate token '" + token);
      // check uniqueness across files
      if (uniqueTypeNames) {
        auto i = uniqueTypeNames->insert(token);
        if (!i.second) {
          dups.emplace_back(*i.first);
          continue;
        }
        good.emplace_back(*i.first);
      } else if (!UniqueNames.insert(token).second)
        error("found globally non-unique entry '" + token + "'");
      _list.push_back(token);
      // _map 'value' starts at 1, i.e., the first kanji has 'frequency 1' (not 0)
      _map[token] = _list.size();
    }
  }
  if (!dups.empty()) {
    if (good.empty())
      error("found " + std::to_string(dups.size()) + " duplicates in " + _name, false);
    else {
      std::cerr << ">>> found " << dups.size() << " duplicates in " << _name << ":";
      for (const auto& i : dups)
        std::cerr << ' ' << i;
      if (createNewUniqueFile) {
        fs::path newFile(file);
        newFile.replace_extension(fs::path("new"));
        std::cerr << "\n>>> saving " << good.size() << " unique entries to: " << newFile.string() << '\n';
        std::ofstream of(newFile);
        for (const auto& i : good)
          of << i << '\n';
      }
    }
  }
}

} // namespace kanji_tools
