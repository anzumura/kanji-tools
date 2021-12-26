#ifndef KANJI_TOOLS_UTILS_COLUMN_FILE_H
#define KANJI_TOOLS_UTILS_COLUMN_FILE_H

#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace kanji_tools {

// 'ColumnFile' is a helper class for loading data from a tab separated text file with a
// header row (containing the column names).
class ColumnFile {
public:
  // 'Column' has a name that must match a column header in the file being processed. The set of
  // columns for a 'ColumnFile' are passed into its constructor and the same column instances are
  // used to get values from each row. A 'Column' can be used across multiple 'ColumnFile' instances.
  class Column {
  public:
    Column(const std::string& name) : _name(name), _number(ColumnFile::getColumnNumber(name)) {}

    bool operator==(const Column& rhs) const { return _number == rhs._number; }

    const std::string& name() const { return _name; }
    int number() const { return _number; }
  private:
    const std::string _name;
    const int _number; // globally unique number per column based on '_name'
  };

  using Columns = std::vector<Column>;

  // 'ColumnFile' will throw an exception if 'p' cannot be opened (or is not a regular file) or
  // if the list of 'columns' doesn't match the first row of the file. Note, the columns in the
  // file can be in a different order than 'columns', but the names must all be found.
  ColumnFile(const std::filesystem::path& p, const Columns& columns);

  // 'nextRow' must be called before using 'get'. An exception is thrown if the next row has too
  // few or too many columns. 'nextRow' returns 'false' when there are no more rows in the file.
  bool nextRow();

  // 'get' returns the value for the given column for the current row. An exception is thrown if
  // 'nextRow' hasn't been called yet or if the given column was not passed in to the constructor.
  const std::string& get(const Column&) const;

  // 'error' throws a 'domain_error' exception with 'what' string made from 'msg' plus '_name'.
  // '_currentRow' is also added if it's not zero.
  void error(const std::string& msg) const;

  int columns() const { return _rowValues.size(); }
  int currentRow() const { return _currentRow; }
  const std::string& name() const { return _name; }
private:
  // 'getColumnNumber' is used by 'Column' class constructor
  static int getColumnNumber(const std::string& name);
  friend Column;

  std::fstream _file;

  // '_name' holds the 'last component name' of the file being processed.
  const std::string _name;

  // '_currentRow' starts at 0 and is incremented each time 'nextRow' is called
  int _currentRow = 0;

  // '_rowValues' is updated each time a new row is processed by 'nextRow'
  std::vector<std::string> _rowValues;

  // '_columnToPosition' maps a column 'number' to the position in _rowValues (starting at 0).
  // This collection is populated by the constructor based on the order that the columns are
  // found while processing the first 'header' row. A vector is used instead of a map to make
  // lookups faster (and avoid string compares, etc.) and the extra space of having a sparse
  // collection should be minimal since columns numbers are shared if they have the same name.
  std::vector<int> _columnToPosition;

  // '_allColumns' is used to globally assign unique numbers to 'Column' instances, i.e., if
  // the column name exists then the same number is used, otherwise a new number is assigned.
  inline static std::map<std::string, int> _allColumns;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_UTILS_COLUMN_FILE_H
