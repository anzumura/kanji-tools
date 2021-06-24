#include <kanji/MBChar.h>

#include <fstream>

namespace kanji {

bool MBChar::getNext(std::string& result, bool onlyMB) {
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

size_t MBCharCount::addFile(const fs::path& file, bool recurse) {
  size_t added = 0;
  if (fs::is_regular_file(file)) {
    std::ifstream f(file);
    std::string line;
    while (std::getline(f, line))
      added += add(line);
  } else if (fs::is_directory(file))
    for (const fs::directory_entry& i : fs::directory_iterator(file))
      added += recurse ? addFile(i.path()) : fs::is_regular_file(i.path()) ? addFile(i.path(), false) : 0;
  return added;
}

} // namespace kanji
