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
  : _file(std::fstream(p)), _fileName(p.string()), _columnToPosition(_allColumns.size() - 1, -1) {}

bool ColumnFile::nextRow() { return false; }

const std::string& ColumnFile::get(const Column& column) const {
  if (column.number() >= _columnToPosition.size())
    error("column '" + column.name() + "' was created after this class was constructed");
  int position = _columnToPosition[column.number()];
  if (position == -1) error("column '" + column.name() + "' is not part of this file");
  return _rowValues[position];
}

void ColumnFile::error(const std::string& msg) const {
  auto errorMsg = msg + " - file: " + _fileName;
  if (_rowCount) errorMsg += ", row: " + std::to_string(_rowCount);
  throw std::domain_error(errorMsg);
}

} // namespace kanji_tools
