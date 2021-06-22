#include <kanji/FileList.h>

#include <fstream>
#include <sstream>

namespace kanji {

namespace fs = std::filesystem;

const char* toString(Levels x) {
  switch (x) {
  case Levels::N1: return "N1";
  case Levels::N2: return "N2";
  case Levels::N3: return "N3";
  case Levels::N4: return "N4";
  case Levels::N5: return "N5";
  default: return "None";
  }
}

FileList::Set FileList::UniqueNames;
FileList::Set FileList::UniqueLevelNames;

fs::path FileList::getRegularFile(const fs::path& dir, const fs::path& file) {
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

FileList::FileList(const fs::path& p, Levels l, bool onePerLine)
  : _name(l != Levels::None ? std::string("JLPT ") + toString(l)
            : onePerLine    ? std::string("Top Frequency")
                            : capitalize(p.stem().string())),
    _level(l) {
  if (!fs::is_regular_file(p)) usage("can't open " + p.string());
  std::ifstream f(p);
  std::string line;
  FileList::List good, dups;
  while (std::getline(f, line)) {
    std::stringstream ss(line);
    auto error = [&](const std::string& s){ usage(s + " - line: '" + line + "', file: " + p.string()); };
    for (std::string token; std::getline(ss, token, ' ');) {
      if (onePerLine) {
        if (token != line) error("got multiple tokens");
      } else if (token.empty() || token == "　")
        continue; // skip empty tokens and 'wide spaces' when processing multiple entries per line
      if (length(token) != 1) error("found token '" + token + "' with length " + std::to_string(length(token)));
      // check uniqueness with file
      if (_map.find(token) != _map.end())
        error("got duplicate token '" + token);
      // check uniqueness across files
      if (l == Levels::None) {
        if (!UniqueNames.insert(token).second)
          error("found globally non-unique entry '" + token + "'");
      } else {
        auto i = UniqueLevelNames.insert(token);
        if (!i.second) {
          dups.emplace_back(*i.first);
          continue;
        }
        good.emplace_back(*i.first);
      }
      _list.push_back(token);
      // _map 'value' starts at 1, i.e., the first kanji has 'frequency 1' (not 0)
      _map[token] = _list.size();
    }
  }
  if (!dups.empty()) {
    std::cerr << ">>> found " << dups.size() << " duplicates in JLPT list " << _name << ":";
    for (const auto& i : dups)
      std::cerr << ' ' << i;
    fs::path newFile(p);
    newFile.replace_extension(fs::path("new"));
    std::cerr << "\n>>> saving " << good.size() << " unique entries to: " << newFile.string() << '\n';
    std::ofstream of(newFile);
    for (const auto& i : good)
      of << i << '\n';
  }
}

} // namespace kanji
