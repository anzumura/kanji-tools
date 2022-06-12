#include <kt_kana/Utf8Char.h>
#include <kt_stats/Utf8Count.h>
#include <kt_utils/BlockRange.h>
#include <kt_utils/Exception.h>

#include <fstream>

namespace kanji_tools {

namespace {

const String OpenWideBracket{"（"}, CloseWideBracket{"）"};
const auto CloseWideBracketSize{CloseWideBracket.size()};

} // namespace

namespace fs = std::filesystem;

const std::wregex Utf8Count::RemoveFurigana{std::wstring{L"(["} + KanjiRange() +
                                            WideLetterRange() + L"]{1})（[" +
                                            KanaRange() + L"]+）"};
const std::wstring Utf8Count::DefaultReplace{L"$1"};

Utf8Count::Utf8Count(
    const OptRegex& find, const std::wstring& replace, std::ostream* debug)
    : _find{find}, _replace{replace}, _debug{debug} {}

size_t Utf8Count::add(const String& s, const OptString& tag) {
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
        static constexpr auto Indent{5};
        const auto count{std::to_string(_replacements)};
        *_debug << "  " << count << " : " << s << '\n'
                << std::setw(static_cast<int>(count.size() + Indent)) << ": "
                << n << '\n';
      }
    }
  }
  Utf8Char c{n};
  size_t added{};
  for (String token; c.next(token);)
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

size_t Utf8Count::addFile(const std::filesystem::path& file, bool addTag,
    bool fileNames, bool recurse) {
  if (!std::filesystem::exists(file))
    throw DomainError{"file not found: " + file.string()};
  return doAddFile(file, addTag, fileNames, recurse);
}

size_t Utf8Count::count(const String& s) const {
  const auto i{_map.find(s)};
  return i != _map.end() ? i->second : 0;
}

const Utf8Count::Map* Utf8Count::tags(const String& s) const {
  const auto i{_tags.find(s)};
  return i != _tags.end() ? &i->second : nullptr;
}

size_t Utf8Count::doAddFile(
    const fs::path& file, bool addTag, bool fileNames, bool recurse) {
  size_t added{};
  const auto fileName{file.filename().string()}; // use final component of path
  const auto tag{addTag ? OptString(fileName) : std::nullopt};
  if (fs::is_regular_file(file)) {
    ++_files;
    added += processFile(file, tag);
  } else if (fs::is_directory(file)) {
    ++_directories;
    for (const auto& i : fs::directory_iterator(file))
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

bool Utf8Count::hasUnclosedBrackets(const String& line) {
  if (const auto open{line.rfind(OpenWideBracket)}; open != String::npos) {
    const auto close{line.rfind(CloseWideBracket)};
    return close == String::npos || close < open;
  }
  return false;
}

size_t Utf8Count::processJoinedLine(
    String& prevLine, const String& line, size_t pos, const OptString& tag) {
  const auto end{pos + CloseWideBracketSize};
  const auto joinedLine{prevLine + line.substr(0, end)};
  // set 'prevLine' to the unprocessed portion of 'line'
  prevLine = line.substr(end);
  return add(joinedLine, tag);
}

size_t Utf8Count::processFile(const fs::path& file, const OptString& tag) {
  if (_find) return processFileWithRegex(file, tag);
  size_t added{};
  String line;
  for (std::fstream f{file}; (std::getline(f, line));) added += add(line, tag);
  return added;
}

size_t Utf8Count::processFileWithRegex(
    const fs::path& file, const OptString& tag) {
  size_t added{};
  std::fstream f{file};
  String line, prevLine;
  for (auto prevUnclosed{false}; std::getline(f, line);
       prevUnclosed = hasUnclosedBrackets(prevLine)) {
    if (!prevLine.empty()) {
      if (prevUnclosed) {
        // if prevLine is unclosed and 'close' comes before 'open' or 'open' is
        // npos ('max size_t') on the current line then process joined lines
        if (const auto close{line.find(CloseWideBracket)};
            close != String::npos && close < line.find(OpenWideBracket)) {
          added += processJoinedLine(prevLine, line, close, tag);
          continue;
        }
      } else if (!line.find(OpenWideBracket)) // line starts with open bracket
        if (const auto close{line.find(CloseWideBracket)};
            close != String::npos) {
          added += processJoinedLine(prevLine, line, close, tag);
          continue;
        }
      // A new open bracket came before 'close' or no 'close' at all on line so
      // give up on trying to balance and just process prevLine.
      added += add(prevLine, tag);
    }
    prevLine = line;
  }
  if (!prevLine.empty()) added += add(prevLine, tag);
  return added;
}

} // namespace kanji_tools
