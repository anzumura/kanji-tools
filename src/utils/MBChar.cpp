#include <kanji_tools/utils/MBChar.h>
#include <kanji_tools/utils/UnicodeBlock.h>

#include <fstream>
#include <iostream>

namespace kanji_tools {

namespace {

const std::string OpenWideBracket("（"), CloseWideBracket("）");
const int CloseWideBracketLength = CloseWideBracket.length();

} // namespace

bool MBChar::next(std::string& result, bool onlyMB) {
  for (; *_location; ++_location) {
    const unsigned char firstOfGroup = *_location;
    if (unsigned char x = firstOfGroup & Mask; !x || x == Bit2) { // not a multi byte character
      if (!onlyMB) {
        result = *_location++;
        return true;
      }
    } else if (MBChar::isValid(_location, false)) {
      // only modify 'result' if '_location' is the start of a valid UTF-8 group
      result = *_location++;
      for (x = Bit2; x && firstOfGroup & x; x >>= 1)
        result += *_location++;
      if (!isVariationSelector(result))
        if (std::string s; doPeek(s, onlyMB, _location, true) && isVariationSelector(s)) {
          result += s;
          _location += 3;
          ++_variants;
        }
      return true;
    } else
      ++_errors;
  }
  return false;
}

bool MBChar::doPeek(std::string& result, bool onlyMB, const char* location, bool internalCall) const {
  for (; *location; ++location) {
    const unsigned char firstOfGroup = *location;
    if (unsigned char x = firstOfGroup & Mask; !x || x == Bit2) { // not a multi byte character
      if (internalCall) return false;
      if (!onlyMB) {
        result = *location;
        return true;
      }
    } else if (MBChar::isValid(location, false)) {
      // only modify 'result' if 'location' is the start of a valid UTF-8 group
      result = *location++;
      for (x = Bit2; x && firstOfGroup & x; x >>= 1)
        result += *location++;
      if (!internalCall && !isVariationSelector(result))
        if (std::string s; doPeek(s, onlyMB, location, true) && isVariationSelector(s)) result += s;
      return true;
    }
  }
  return false;
}

namespace fs = std::filesystem;

const std::wregex MBCharCount::RemoveFurigana(std::wstring(L"([") + KanjiRange + L"]{1})（[" + KanaRange + L"]+）");
const std::wstring MBCharCount::DefaultReplace(L"$1");

size_t MBCharCount::add(const std::string& s, const std::string& tag) {
  std::string n = s;
  if (_find) {
    n = toUtf8(std::regex_replace(fromUtf8(s), *_find, _replace));
    if (n != s) {
      ++_replaceCount;
      if (!tag.empty() && tag != _lastReplaceTag) {
        if (_debug) std::cout << ">>> Tag: " << tag << '\n';
        _lastReplaceTag = tag;
      }
      if (_debug) {
        auto count = std::to_string(_replaceCount);
        std::cout << count << " : " << s << '\n' << std::setw(count.length() + 3) << ": " << n << '\n';
      }
    }
  }
  MBChar c(n);
  size_t added = 0;
  for (std::string token; c.next(token);)
    if (allowAdd(token)) {
      ++_map[token];
      ++added;
      if (!tag.empty()) ++_tags[token][tag];
    }
  _errors += c.errors();
  _variants += c.variants();
  return added;
}

size_t MBCharCount::doAddFile(const fs::path& file, bool addTag, bool fileNames, bool recurse) {
  size_t added = 0;
  std::string tag = file.filename().string(); // only use the final component of the path
  if (fs::is_regular_file(file)) {
    ++_files;
    if (_find)
      added = balanceBrackets(file, addTag, tag);
    else {
      std::ifstream f(file);
      for (std::string line; std::getline(f, line);)
        added += addTag ? add(line, tag) : add(line);
    }
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

bool MBCharCount::isOpenEnded(const std::string& line) {
  if (auto open = line.rfind(OpenWideBracket); open != std::string::npos) {
    auto close = line.rfind(CloseWideBracket);
    return (close == std::string::npos || close < open);
  }
  return false;
}

bool MBCharCount::processPartial(std::string& prevLine, size_t pos, const std::string& line, size_t& added, bool addTag,
                                 const std::string& tag) {
  const auto end = pos + CloseWideBracketLength;
  const std::string s = prevLine + line.substr(0, end);
  added += addTag ? add(s, tag) : add(s);
  // set 'prevLine' to the unprocessed portion of 'line'
  prevLine = line.substr(end);
  return isOpenEnded(prevLine);
}

size_t MBCharCount::balanceBrackets(const std::filesystem::path& file, bool addTag, const std::string& tag) {
  size_t added = 0;
  std::string line, prevLine;
  bool prevOpenEnded = false;

  for (std::ifstream f(file); std::getline(f, line);) {
    if (prevLine.empty()) {
      // special case for first line - don't process in case next line stars with open bracket.
      prevLine = line;
      prevOpenEnded = isOpenEnded(prevLine);
      continue;
    } else if (prevOpenEnded) {
      if (auto close = line.find(CloseWideBracket); close != std::string::npos) {
        if (auto open = line.find(OpenWideBracket); close < open) {
          prevOpenEnded = processPartial(prevLine, close, line, added, addTag, tag);
          continue;
        }
      }
    } else if (auto open = line.find(OpenWideBracket); open == 0)
      // special case for line starting with open bracket
      if (auto close = line.find(CloseWideBracket); close != std::string::npos) {
        prevOpenEnded = processPartial(prevLine, close, line, added, addTag, tag);
        continue;
      }
    // A new open bracket came before 'close' or no 'close' at all on line so give up on
    // trying to balance and just process prevLine.
    added += addTag ? add(prevLine, tag) : add(prevLine);
    prevLine = line;
    prevOpenEnded = isOpenEnded(prevLine);
  }
  if (!prevLine.empty()) added += addTag ? add(prevLine, tag) : add(prevLine);
  return added;
}

} // namespace kanji_tools
