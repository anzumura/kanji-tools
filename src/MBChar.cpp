#include <kanji/MBChar.h>

#include <fstream>
#include <iostream>

namespace kanji {

bool MBChar::next(std::string& result, bool onlyMB) {
  for (; *_location; ++_location) {
    const unsigned char firstOfGroup = *_location;
    unsigned char x = firstOfGroup & Mask;
    if (!x || x == Bit2) { // not a multi byte character
      if (!onlyMB) {
        result = *_location++;
        return true;
      }
    } else if (x != Bit1) { // shouldn't be a continuation, but check to be safe
      result = *_location++;
      for (x = Bit2; x && firstOfGroup & x; x >>= 1)
        result += *_location++;
      return true;
    }
  }
  return false;
}

namespace fs = std::filesystem;

size_t MBCharCount::add(const std::string& s, const std::string& tag) {
  std::string n = s;
  if (_find.has_value()) {
    static int count;
    n = toUtf8(std::regex_replace(fromUtf8(s), *_find, _replace));
    if (_debug && n != s) std::cout << ++count << " Before: " << s << '\n' << count << "  After: " << n << '\n';
  }
  MBChar c(n);
  size_t added = 0;
  for (std::string token; c.next(token);)
    if (allowAdd(token)) {
      ++_map[token];
      ++added;
      if (!tag.empty()) ++_tags[token][tag];
    }
  return added;
}

size_t MBCharCount::doAddFile(const fs::path& file, bool addTag, bool fileNames, bool recurse) {
  size_t added = 0;
  std::string tag = file.filename().string(); // only use the final component of the path
  if (fs::is_regular_file(file)) {
    ++_files;
    std::ifstream f(file);
    std::string line;
    while (std::getline(f, line))
      added += addTag ? add(line, tag) : add(line);
  } else if (fs::is_directory(file)) {
    ++_directories;
    for (fs::directory_entry i : fs::directory_iterator(file))
      added += recurse                  ? doAddFile(i.path(), addTag, fileNames)
        : fs::is_regular_file(i.path()) ? doAddFile(i.path(), addTag, fileNames, false)
                                        : 0;
  } else // skip if not a regular file or directory
    return 0;
  if (fileNames) added += addTag ? add(tag, tag) : add(tag);
  return added;
}

} // namespace kanji
