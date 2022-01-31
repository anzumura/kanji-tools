#include <kanji_tools/utils/MBChar.h>
#include <kanji_tools/utils/UnicodeBlock.h>

#include <fstream>
#include <iostream>

namespace kanji_tools {

namespace {

const std::string OpenWideBracket("（"), CloseWideBracket("）");
const auto CloseWideBracketLength = CloseWideBracket.length();

} // namespace

MBChar::Results MBChar::validateUtf8(const char* s, bool checkLengthOne) {
  if (!s || !(*s & Bit1)) return Results::NotMBChar;
  if ((*s & TwoBits) == Bit1) return Results::ContinuationByte;
  auto* u = reinterpret_cast<const unsigned char*>(s);
  const unsigned byte1 = *u;
  if ((*++u & TwoBits) != Bit1) return Results::MBCharMissingBytes; // second byte didn't start with '10'
  if (byte1 & Bit3) {
    const unsigned byte2 = *u ^ Bit1;                                 // last 6 bits of the second byte
    if ((*++u & TwoBits) != Bit1) return Results::MBCharMissingBytes; // third byte didn't start with '10'
    if (byte1 & Bit4) {
      if (byte1 & Bit5) return Results::MBCharTooLong;                  // UTF-8 can only have up to 4 bytes
      const unsigned byte3 = *u ^ Bit1;                                 // last 6 bits of the third byte
      if ((*++u & TwoBits) != Bit1) return Results::MBCharMissingBytes; // fourth byte didn't start with '10'
      if (((byte1 ^ FourBits) << 18) + (byte2 << 12) + (byte3 << 6) + (*u ^ Bit1) <= 0xffffU)
        return Results::Overlong; // overlong 4 byte encoding
    } else if (((byte1 ^ ThreeBits) << 12) + (byte2 << 6) + (*u ^ Bit1) <= 0x7ffU)
      return Results::Overlong; // overlong 3 byte encoding
  } else if ((byte1 ^ TwoBits) < 2)
    return Results::Overlong; // overlong 2 byte encoding
  return !checkLengthOne || !*++u ? Results::Valid : Results::StringTooLong;
}

bool MBChar::next(std::string& result, bool onlyMB) {
  for (; *_location; ++_location) {
    const unsigned char firstOfGroup = *_location;
    if (unsigned char x = firstOfGroup & TwoBits; !x || x == Bit2) { // not a multi byte character
      if (!onlyMB) {
        result = *_location++;
        return true;
      }
    } else if (MBChar::isValid(_location, false)) {
      // only modify 'result' if '_location' is the start of a valid UTF-8 group
      result = *_location++;
      for (x = Bit2; x && firstOfGroup & x; x >>= 1) result += *_location++;
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
    if (unsigned char x = firstOfGroup & TwoBits; !x || x == Bit2) { // not a multi byte character
      if (internalCall) return false;
      if (!onlyMB) {
        result = *location;
        return true;
      }
    } else if (MBChar::isValid(location, false)) {
      // only modify 'result' if 'location' is the start of a valid UTF-8 group
      result = *location++;
      for (x = Bit2; x && firstOfGroup & x; x >>= 1) result += *location++;
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

int MBCharCount::add(const std::string& s, const OptString& tag) {
  auto n = s;
  if (_find) {
    n = toUtf8(std::regex_replace(fromUtf8(s), *_find, _replace));
    if (n != s) {
      ++_replaceCount;
      if (tag && tag != _lastReplaceTag) {
        if (_debug) std::cout << ">>> Tag: " << *tag << '\n';
        _lastReplaceTag = *tag;
      }
      if (_debug) {
        const auto count = std::to_string(_replaceCount);
        std::cout << count << " : " << s << '\n' << std::setw(count.length() + 3) << ": " << n << '\n';
      }
    }
  }
  MBChar c(n);
  auto added = 0;
  for (std::string token; c.next(token);)
    if (allowAdd(token)) {
      ++_map[token];
      ++added;
      if (tag) ++_tags[token][*tag];
    }
  _errors += c.errors();
  _variants += c.variants();
  return added;
}

int MBCharCount::doAddFile(const fs::path& file, bool addTag, bool fileNames, bool recurse) {
  auto added = 0;
  const auto fileName = file.filename().string(); // only use the final component of the path
  const auto tag = addTag ? OptString(fileName) : std::nullopt;
  if (fs::is_regular_file(file)) {
    ++_files;
    added += processFile(file, tag);
  } else if (fs::is_directory(file)) {
    ++_directories;
    for (fs::directory_entry i : fs::directory_iterator(file))
      added += recurse                  ? doAddFile(i.path(), addTag, fileNames)
        : fs::is_regular_file(i.path()) ? doAddFile(i.path(), addTag, fileNames, false)
                                        : 0;
  } else // skip if not a regular file or directory
    return 0;
  if (fileNames) added += add(fileName, tag);
  return added;
}

bool MBCharCount::hasUnclosedBrackets(const std::string& line) {
  if (const auto open = line.rfind(OpenWideBracket); open != std::string::npos) {
    const auto close = line.rfind(CloseWideBracket);
    return close == std::string::npos || close < open;
  }
  return false;
}

int MBCharCount::processJoinedLine(std::string& prevLine, const std::string& line, int pos, const OptString& tag) {
  const auto end = pos + CloseWideBracketLength;
  const auto joinedLine = prevLine + line.substr(0, end);
  // set 'prevLine' to the unprocessed portion of 'line'
  prevLine = line.substr(end);
  return add(joinedLine, tag);
}

int MBCharCount::processFile(const fs::path& file, const OptString& tag) {
  auto added = 0;
  std::string line;
  if (std::fstream f(file); _find) {
    std::string prevLine;
    for (auto prevUnclosed = false; std::getline(f, line); prevUnclosed = hasUnclosedBrackets(prevLine)) {
      if (prevLine.empty()) {
        // case for first line - don't process in case next line stars with open bracket.
        prevLine = line;
        continue;
      } else if (prevUnclosed) {
        // case for previous line having unclosed brackets
        if (const auto close = line.find(CloseWideBracket); close != std::string::npos)
          if (const auto open = line.find(OpenWideBracket); close < open) {
            added += processJoinedLine(prevLine, line, close, tag);
            continue;
          }
      } else if (const auto open = line.find(OpenWideBracket); open == 0)
        // case for line starting with open bracket
        if (const auto close = line.find(CloseWideBracket); close != std::string::npos) {
          added += processJoinedLine(prevLine, line, close, tag);
          continue;
        }
      // A new open bracket came before 'close' or no 'close' at all on line so give up on
      // trying to balance and just process prevLine.
      added += add(prevLine, tag);
      prevLine = line;
    }
    if (!prevLine.empty()) added += add(prevLine, tag);
  } else
    while (std::getline(f, line)) added += add(line, tag);
  return added;
}

} // namespace kanji_tools
