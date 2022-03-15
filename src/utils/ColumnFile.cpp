#include <kanji_tools/utils/ColumnFile.h>

#include <set>
#include <sstream>

namespace kanji_tools {

namespace {

constexpr auto ColNotFound = std::numeric_limits<size_t>::max();

} // namespace

namespace fs = std::filesystem;

size_t ColumnFile::getColumnNumber(const std::string& name) {
  const auto i = _allColumns.find(name);
  if (i == _allColumns.end()) return _allColumns[name] = _allColumns.size();
  return i->second;
}

ColumnFile::ColumnFile(const fs::path& p, const Columns& columns,
                       char delimiter)
    : _file(std::fstream(p)), _delimiter(delimiter),
      _name(p.filename().string()), _rowValues(columns.size()),
      _columnToPosition(_allColumns.size(), ColNotFound) {
  if (columns.empty()) error("must specify at least one column");
  if (!fs::exists(p)) error("doesn't exist");
  if (!fs::is_regular_file(p)) error("not regular file");
  if (std::string headerRow; std::getline(_file, headerRow)) {
    ColNames colNames;
    for (auto& c : columns)
      if (!colNames.emplace(c.name(), c).second)
        error("duplicate column '" + c.name() + "'");
    processHeaderRow(headerRow, colNames);
    verifyHeaderColumns(colNames);
  } else
    error("missing header row");
}

void ColumnFile::processHeaderRow(const std::string& row, ColNames& colNames) {
  size_t pos{};
  std::set<std::string> foundCols;
  std::string header;
  for (std::stringstream ss(row); std::getline(ss, header, _delimiter); ++pos) {
    if (foundCols.contains(header)) error("duplicate header '" + header + "'");
    const auto i = colNames.find(header);
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
    std::string msg;
    for (auto& i : colNames) {
      if (!msg.empty()) msg += ',';
      msg += " '" + i.first + "'";
    }
    error(std::to_string(colNames.size()) + " columns not found:" + msg);
  }
}

bool ColumnFile::nextRow() {
  if (std::string line; std::getline(_file, line)) {
    ++_currentRow;
    size_t i{};
    std::string field;
    for (std::stringstream ss(line); std::getline(ss, field, _delimiter); ++i) {
      if (i == _rowValues.size()) error("too many columns");
      _rowValues[i] = field;
    }
    // 'getline' will return failure if it only reads a delimiter and then
    // reaches the end of input so need a special case for handing an empty
    // final column.
    if (i == _rowValues.size() - 1 &&
        (i ? line.ends_with(_delimiter) : line.empty()))
      _rowValues[_rowValues.size() - 1] = EmptyString;
    else if (i < _rowValues.size())
      error("not enough columns");
    return true;
  }
  return false;
}

const std::string& ColumnFile::get(const Column& column) const {
  if (!_currentRow) error("'nextRow' must be called before calling 'get'");
  if (column.number() >= _columnToPosition.size())
    error("unrecognized column '" + column.name() + "'");
  const auto pos = _columnToPosition[column.number()];
  if (pos == ColNotFound) error("invalid column '" + column.name() + "'");
  return _rowValues[pos];
}

size_t ColumnFile::getSize(const Column& column) const {
  auto& s = get(column);
  try {
    return std::stoul(s);
  } catch (...) {
    error("failed to convert to size_t", column, s);
  }
  __builtin_unreachable(); // 'error' function always throws an exception
}

ColumnFile::OptSize ColumnFile::getOptSize(const Column& column) const {
  auto& s = get(column);
  if (s.empty()) return {};
  try {
    return std::stoi(s);
  } catch (...) {
    error("failed to convert to size_t", column, s);
  }
  __builtin_unreachable(); // 'error' function always throws an exception
}

bool ColumnFile::getBool(const Column& column) const {
  auto& s = get(column);
  if (s.size() == 1) switch (s[0]) {
    case 'Y':
    case 'T': return true;
    case 'N':
    case 'F': return false;
    }
  if (!s.empty()) error("failed to convert to bool", column, s);
  return false;
}

char32_t ColumnFile::getWChar(const Column& column,
                              const std::string& s) const {
  if (s.size() < 4 || s.size() > 5)
    error("failed to convert to char32_t, size must be 4 or 5", column, s);
  for (const char c : s)
    if (c < '0' || c > 'F' || (c < 'A' && c > '9'))
      error("failed to convert to char32_t, invalid hex", column, s);
  return static_cast<char32_t>(std::strtol(s.c_str(), nullptr, 16));
}

} // namespace kanji_tools
