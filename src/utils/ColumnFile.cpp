#include <kanji_tools/utils/ColumnFile.h>

#include <set>
#include <sstream>

namespace kanji_tools {

namespace fs = std::filesystem;

int ColumnFile::getColumnNumber(const std::string& name) {
  auto i = _allColumns.find(name);
  if (i == _allColumns.end()) {
    int n = _allColumns.size();
    _allColumns[name] = n;
    return n;
  }
  return i->second;
}

ColumnFile::ColumnFile(const fs::path& p, const Columns& columns)
  : _file(std::fstream(p)), _name(p.filename().string()), _rowValues(columns.size()),
    _columnToPosition(_allColumns.size(), -1) {
  if (!fs::exists(p)) error("doesn't exist");
  if (!fs::is_regular_file(p)) error("not regular file");
  if (std::string headerRow; std::getline(_file, headerRow)) {
    ColNames colNames;
    for (auto& c : columns)
      if (!colNames.insert(std::make_pair(c.name(), c)).second) error("duplicate column '" + c.name() + "'");
    processHeaderRow(headerRow, colNames);
    verifyHeaderColumns(colNames);
  } else
    error("missing header row");
}

void ColumnFile::processHeaderRow(const std::string& headerRow, ColNames& colNames) {
  int pos = 0;
  std::set<std::string> foundCols;
  std::string header;
  for (std::stringstream ss(headerRow); std::getline(ss, header, '\t'); ++pos) {
    if (foundCols.contains(header)) error("duplicate header '" + header + "'");
    auto i = colNames.find(header);
    if (i == colNames.end()) error("unrecognized header '" + header + "'");
    _columnToPosition[i->second.number()] = pos;
    foundCols.insert(header);
    colNames.erase(i);
  }
}

void ColumnFile::verifyHeaderColumns(const ColNames& colNames) const {
  if (colNames.size() == 1) error("column '" + (*colNames.begin()).first + "' not found");
  if (colNames.size() > 1) {
    std::string msg;
    for (auto i : colNames) {
      if (!msg.empty()) msg += ',';
      msg += " '" + i.first + "'";
    }
    error(std::to_string(colNames.size()) + " columns not found:" + msg);
  }
}

bool ColumnFile::nextRow() {
  if (std::string line; std::getline(_file, line)) {
    ++_currentRow;
    int pos = 0;
    std::string field;
    for (std::stringstream ss(line); std::getline(ss, field, '\t'); ++pos) {
      if (pos == _rowValues.size()) error("too many columns");
      _rowValues[pos] = field;
    }
    // 'getline' will return failure if it only reads a delimiter and then reaches the end of input
    // so need a special case for handing an empty final column.
    if (pos == _rowValues.size() - 1 && (pos ? line.ends_with("\t") : line.empty()))
      _rowValues[_rowValues.size() - 1] = EmptyString;
    else if (pos < _rowValues.size())
      error("not enough columns");
    return true;
  }
  return false;
}

const std::string& ColumnFile::get(const Column& column) const {
  if (!_currentRow) error("'nextRow' must be called before calling 'get'");
  if (column.number() >= _columnToPosition.size()) error("unrecognized column '" + column.name() + "'");
  int position = _columnToPosition[column.number()];
  if (position == -1) error("invalid column '" + column.name() + "'");
  return _rowValues[position];
}

int ColumnFile::getInt(const Column& column) const {
  const std::string& s = get(column);
  int result;
  try {
    result = std::stoi(s);
  } catch (...) {
    error("failed to convert to int", column, s);
  }
  return result;
}

ColumnFile::OptInt ColumnFile::getOptInt(const Column& column) const {
  const std::string& s = get(column);
  if (s.empty()) return std::nullopt;
  int result;
  try {
    result = std::stoi(s);
  } catch (...) {
    error("failed to convert to int", column, s);
  }
  return result;
}

bool ColumnFile::getBool(const Column& column) const {
  const std::string& s = get(column);
  if (s.length() == 1) switch (s[0]) {
    case 'Y':
    case 'T': return true;
    case 'N':
    case 'F': return false;
    }
  if (!s.empty()) error("failed to convert to bool", column, s);
  return false;
}

wchar_t ColumnFile::getWChar(const Column& column, const std::string& s) const {
  if (s.length() < 4 || s.length() > 5) error("failed to convert to wchar_t, length must be 4 or 5", column, s);
  for (char c : s)
    if (c < '0' || c > 'F' || (c < 'A' && c > '9')) error("failed to convert to wchar_t, invalid hex", column, s);
  return std::strtol(s.c_str(), nullptr, 16);
}

} // namespace kanji_tools
