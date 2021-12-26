#include <kanji_tools/utils/ColumnFile.h>

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
  std::string line;
  if (!std::getline(_file, line)) error("missing header row");
  std::map<std::string, Column> colNames;
  for (auto& c : columns)
    if (!colNames.insert(std::make_pair(c.name(), c)).second) error("duplicate column '" + c.name() + "'");
  int pos = 0;
  for (std::stringstream ss(line); std::getline(ss, line, '\t'); ++pos) {
    auto i = colNames.find(line);
    if (i == colNames.end()) error("unrecognized header '" + line + "'");
    _columnToPosition[i->second.number()] = pos;
    colNames.erase(i);
  }
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
    for (std::stringstream ss(line); std::getline(ss, line, '\t'); ++pos) {
      if (pos == _rowValues.size()) error("too many columns");
      _rowValues[pos] = line;
    }
    if (pos != _rowValues.size()) error("not enough columns");
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

void ColumnFile::error(const std::string& msg) const {
  auto errorMsg = msg + " - file: " + _name;
  if (_currentRow) errorMsg += ", row: " + std::to_string(_currentRow);
  throw std::domain_error(errorMsg);
}

} // namespace kanji_tools
