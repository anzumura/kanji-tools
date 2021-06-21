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

FileList::FileList(const fs::path& p, Levels l)
  : _name(l == Levels::None ? std::string("Top Frequency") : std::string("JLPT ") + toString(l)), _level(l) {
  if (!fs::is_regular_file(p)) usage("can't open " + p.string());
  int count = 0;
  std::ifstream f(p);
  std::string line;
  FileList::List good, dups;
  while (std::getline(f, line)) {
    assert(_map.find(line) == _map.end());
    if (l != Levels::None) {
      auto i = UniqueNames.insert(line);
      if (!i.second) {
        dups.emplace_back(*i.first);
        continue;
      }
      good.emplace_back(*i.first);
    }
    _list.emplace_back(line);
    // map value count starts at 1, i.e., the first kanji has 'frequency 1' (not 0)
    _map[line] = ++count;
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
