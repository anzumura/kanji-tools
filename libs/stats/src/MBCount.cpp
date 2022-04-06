#include <kanji_tools/kana/MBChar.h>
#include <kanji_tools/stats/MBCount.h>
#include <kanji_tools/utils/UnicodeBlock.h>

#include <fstream>
#include <iostream>

namespace kanji_tools {

namespace {

const std::string OpenWideBracket{"（"}, CloseWideBracket{"）"};
const auto CloseWideBracketSize{CloseWideBracket.size()};

} // namespace

namespace fs = std::filesystem;

const std::wregex MBCount::RemoveFurigana{
    std::wstring{L"(["} + KanjiRange() + L"]{1})（[" + KanaRange() + L"]+）"};
const std::wstring MBCount::DefaultReplace{L"$1"};

size_t MBCount::add(const std::string& s, const OptString& tag) {
  auto n{s};
  if (_find) {
    n = toUtf8(std::regex_replace(fromUtf8ToWstring(s), *_find, _replace));
    if (n != s) {
      ++_replacements;
      if (tag && tag != _lastReplaceTag) {
        if (_debug) *_debug << "Tag '" << *tag << "'\n";
        _lastReplaceTag = *tag;
      }
      if (_debug) {
        const auto count{std::to_string(_replacements)};
        *_debug << "  " << count << " : " << s << '\n'
                << std::setw(static_cast<int>(count.size() + 5)) << ": " << n
                << '\n';
      }
    }
  }
  MBChar c{n};
  size_t added{};
  for (std::string token; c.next(token);)
    if (allowAdd(token)) {
      ++_map[token];
      ++added;
      if (tag) ++_tags[token][*tag];
    }
  _errors += c.errors();
  _variants += c.variants();
  _combiningMarks += c.combiningMarks();
  return added;
}

size_t MBCount::doAddFile(
    const fs::path& file, bool addTag, bool fileNames, bool recurse) {
  size_t added{};
  const auto fileName{file.filename().string()}; // use final component of path
  const auto tag{addTag ? OptString(fileName) : std::nullopt};
  if (fs::is_regular_file(file)) {
    ++_files;
    added += processFile(file, tag);
  } else if (fs::is_directory(file)) {
    ++_directories;
    for (fs::directory_entry i : fs::directory_iterator(file))
      // skip symlinks for now when potentially recursing
      added += i.is_symlink() ? 0
               : recurse      ? doAddFile(i.path(), addTag, fileNames)
               : i.is_regular_file()
                   ? doAddFile(i.path(), addTag, fileNames, false)
                   : 0;
  } else // skip if not a regular file or directory
    return 0;
  if (fileNames) added += add(fileName, tag);
  return added;
}

bool MBCount::hasUnclosedBrackets(const std::string& line) {
  if (const auto open{line.rfind(OpenWideBracket)}; open != std::string::npos) {
    const auto close{line.rfind(CloseWideBracket)};
    return close == std::string::npos || close < open;
  }
  return false;
}

size_t MBCount::processJoinedLine(std::string& prevLine,
    const std::string& line, size_t pos, const OptString& tag) {
  const auto end{pos + CloseWideBracketSize};
  const auto joinedLine{prevLine + line.substr(0, end)};
  // set 'prevLine' to the unprocessed portion of 'line'
  prevLine = line.substr(end);
  return add(joinedLine, tag);
}

size_t MBCount::processFile(const fs::path& file, const OptString& tag) {
  size_t added{};
  std::string line;
  if (std::fstream f{file}; _find) {
    std::string prevLine;
    for (auto prevUnclosed{false}; std::getline(f, line);
         prevUnclosed = hasUnclosedBrackets(prevLine)) {
      if (prevLine.empty()) {
        // don't process in case next line stars with open bracket
        prevLine = line;
        continue;
      } else if (prevUnclosed) {
        // case for previous line having unclosed brackets
        if (const auto close{line.find(CloseWideBracket)};
            close != std::string::npos)
          if (const auto open{line.find(OpenWideBracket)}; close < open) {
            added += processJoinedLine(prevLine, line, close, tag);
            continue;
          }
      } else if (const auto open{line.find(OpenWideBracket)}; !open)
        // case for line starting with open bracket
        if (const auto close{line.find(CloseWideBracket)};
            close != std::string::npos) {
          added += processJoinedLine(prevLine, line, close, tag);
          continue;
        }
      // A new open bracket came before 'close' or no 'close' at all on line so
      // give up on trying to balance and just process prevLine.
      added += add(prevLine, tag);
      prevLine = line;
    }
    if (!prevLine.empty()) added += add(prevLine, tag);
  } else
    while (std::getline(f, line)) added += add(line, tag);
  return added;
}

} // namespace kanji_tools
