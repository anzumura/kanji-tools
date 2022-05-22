#include <kanji_tools/utils/ColumnFile.h>

#include <cassert>
#include <set>
#include <sstream>

namespace kanji_tools {

namespace {

constexpr auto ColNotFound{std::numeric_limits<size_t>::max()};

} // namespace

ColumnFile::Column::Column(const String& name)
    : _name{name}, _number{ColumnFile::getColumnNumber(name)} {}

bool ColumnFile::Column::operator==(const Column& rhs) const {
  return _number == rhs._number;
}

size_t ColumnFile::getColumnNumber(const String& name) {
  const auto i{_allColumns.find(name)};
  if (i == _allColumns.end()) return _allColumns[name] = _allColumns.size();
  return i->second;
}

ColumnFile::ColumnFile(const Path& p, const Columns& columns, char delim)
    : _file{std::fstream(p)}, _delimiter{delim},
      _fileName{p.filename().string()}, _rowValues{columns.size()},
      _columnToPosition(_allColumns.size(), ColNotFound) {
  assert(_columnToPosition.size() == _allColumns.size()); // need () ctor
  if (columns.empty()) error("must specify at least one column");
  if (!std::filesystem::exists(p)) error("doesn't exist");
  if (!std::filesystem::is_regular_file(p)) error("not regular file");
  if (String headerRow; std::getline(_file, headerRow)) {
    ColNames colNames;
    for (auto& c : columns)
      if (!colNames.emplace(c.name(), c).second)
        error("duplicate column '" + c.name() + "'");
    processHeaderRow(headerRow, colNames);
    verifyHeaderColumns(colNames);
  } else
    error("missing header row");
}

void ColumnFile::processHeaderRow(const String& row, ColNames& colNames) {
  size_t pos{};
  std::set<String> foundCols;
  String header;
  for (std::stringstream ss{row}; std::getline(ss, header, _delimiter); ++pos) {
    if (foundCols.contains(header)) error("duplicate header '" + header + "'");
    const auto i{colNames.find(header)};
    if (i == colNames.end()) error("unrecognized header '" + header + "'");
    _columnToPosition[i->second.number()] = pos;
    foundCols.insert(header);
    colNames.erase(i);
  }
}

void ColumnFile::verifyHeaderColumns(const ColNames& colNames) const {
  if (colNames.size() == 1)
    error("column '" + (*colNames.begin()).first + "' not found");
  if (colNames.size() > 1) {
    String msg;
    for (auto& i : colNames) {
      if (!msg.empty()) msg += ',';
      msg += " '" + i.first + "'";
    }
    error(std::to_string(colNames.size()) + " columns not found:" + msg);
  }
}

bool ColumnFile::nextRow() {
  if (String line; std::getline(_file, line)) {
    ++_currentRow;
    size_t i{};
    String field;
    for (std::stringstream ss{line}; std::getline(ss, field, _delimiter); ++i) {
      if (i == _rowValues.size()) error("too many columns");
      _rowValues[i] = field;
    }
    // the above call to 'getline' returns false when there's no more data, but
    // it also returns false if it only reads a delimiter and then reaches the
    // end of input so need a special case for an empty final column
    if (i == _rowValues.size() - 1 &&
        (i ? line.ends_with(_delimiter) : line.empty()))
      _rowValues[_rowValues.size() - 1] = EmptyString;
    else if (i < _rowValues.size())
      error("not enough columns");
    return true;
  }
  return false;
}

const String& ColumnFile::get(const Column& column) const {
  if (!_currentRow) error("'nextRow' must be called before calling 'get'");
  if (column.number() >= _columnToPosition.size())
    error("unrecognized column '" + column.name() + "'");
  const auto pos{_columnToPosition[column.number()]};
  if (pos == ColNotFound) error("invalid column '" + column.name() + "'");
  return _rowValues[pos];
}

bool ColumnFile::isEmpty(const Column& c) const { return get(c).empty(); }

ColumnFile::ULong ColumnFile::getULong(const Column& c, ULong max) const {
  return processULong(get(c), c, max);
}

ColumnFile::OptULong ColumnFile::getOptULong(const Column& c, ULong max) const {
  auto& s{get(c)};
  if (s.empty()) return {};
  return processULong(s, c, max);
}

ColumnFile::ULong ColumnFile::processULong(
    const String& s, const Column& column, ULong max) const {
  ULong i{};
  try {
    i = std::stoul(s);
  } catch (...) {
    error("failed to convert to unsigned long", column, s);
  }
  if (max && max < i)
    error("exceeded max value of " + std::to_string(max), column, s);
  return i;
}

bool ColumnFile::getBool(const Column& column) const {
  auto& s{get(column)};
  if (s.size() == 1) switch (s[0]) {
    case 'Y':
    case 'T': return true;
    case 'N':
    case 'F': return false;
    }
  if (!s.empty()) error("failed to convert to bool", column, s);
  return false;
}

Code ColumnFile::getChar32(const Column& column, const String& s) const {
  if (s.size() < UnicodeStringMinSize || s.size() > UnicodeStringMaxSize)
    error("failed to convert to Code, size must be 4 or 5", column, s);
  // want hex with capitals so can't use 'std::ishexnumber'
  if (std::any_of(s.begin(), s.end(),
          [](auto i) { return i < '0' || i > 'F' || (i < 'A' && i > '9'); }))
    error("failed to convert to Code, invalid hex", column, s);
  return static_cast<Code>(std::stoi(s, nullptr, HexDigits));
}

Code ColumnFile::getChar32(const Column& c) const {
  return getChar32(c, get(c));
}

void ColumnFile::error(const String& msg) const {
  throw std::domain_error(errorMsg(msg));
}

void ColumnFile::error(
    const String& msg, const Column& c, const String& s) const {
  throw std::domain_error{
      errorMsg(msg) + ", column: '" + c.name() + "', value: '" + s + "'"};
}

String ColumnFile::errorMsg(const String& msg) const {
  auto result{msg + " - file: " + _fileName};
  if (_currentRow) result += ", row: " + std::to_string(_currentRow);
  return result;
}

} // namespace kanji_tools
