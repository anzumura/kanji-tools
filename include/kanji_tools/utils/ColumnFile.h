#ifndef KANJI_TOOLS_UTILS_COLUMN_FILE_H
#define KANJI_TOOLS_UTILS_COLUMN_FILE_H

#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace kanji_tools {

// 'ColumnFile' is a helper class for loading data from a delimiter (defaults to tab) separated
// text file with a header row (containing the column names).
class ColumnFile {
public:
  static inline const std::string EmptyString;

  // 'Column' has a name that must match a column header in the file being processed. The set of
  // columns for a 'ColumnFile' are passed into its constructor and the same column instances are
  // used to get values from each row. A 'Column' can be used across multiple 'ColumnFile' instances.
  class Column {
  public:
    Column(const std::string& name) : _name(name), _number(ColumnFile::getColumnNumber(name)) {}

    [[nodiscard]] auto operator==(const Column& rhs) const { return _number == rhs._number; }

    [[nodiscard]] auto& name() const { return _name; }
    [[nodiscard]] auto number() const { return _number; }
  private:
    std::string _name;
    size_t _number; // globally unique number per column based on '_name'
  };

  using Columns = std::vector<Column>;
  using OptInt = std::optional<int>;

  // 'ColumnFile' will throw an exception if 'p' cannot be opened (or is not a regular file) or
  // if the list of 'columns' doesn't match the first row of the file. Note, the columns in the
  // file can be in a different order than 'columns', but the names must all be found.
  ColumnFile(const std::filesystem::path& p, const Columns& columns, char delimiter = '\t');

  // 'nextRow' must be called before using 'get'. An exception is thrown if the next row has too
  // few or too many columns. 'nextRow' returns 'false' when there are no more rows in the file.
  bool nextRow();

  // 'get' returns the value for the given column for the current row. An exception is thrown if
  // 'nextRow' hasn't been called yet or if the given column was not passed in to the constructor.
  const std::string& get(const Column&) const;

  [[nodiscard]] auto isEmpty(const Column& column) const { return get(column).empty(); }

  // 'getInt' convert to 'int' or calls 'error'
  int getInt(const Column&) const;

  // 'getOptInt' returns std::nullopt if column is empty or returns an optional int (or calls 'error')
  OptInt getOptInt(const Column&) const;

  // 'getBool' converts 'Y' or 'T' to true, 'N' or 'F' (or empty) to false or calls 'error'
  bool getBool(const Column&) const;

  // 'getWChar' overload for a specific value 's' (can be used for columns with multiple values)
  char32_t getWChar(const Column&, const ::std::string& s) const;

  // 'getWchar' converts from Unicode (4 or 5 hex code) or calls 'error'
  auto getWChar(const Column& c) const { return getWChar(c, get(c)); }

  // 'error' throws a 'domain_error' exception with 'what' string made from 'msg' plus '_name'.
  // '_currentRow' is also added if it's not zero.
  void error(const std::string& msg) const { throw std::domain_error(errorMsg(msg)); }

  // 'error' overload for reporting a problem with a specific value for a column
  void error(const std::string& msg, const Column& c, const std::string& s) const {
    throw std::domain_error(errorMsg(msg) + ", column: '" + c.name() + "', value: '" + s + "'");
  }

  [[nodiscard]] auto columns() const { return _rowValues.size(); }
  [[nodiscard]] auto currentRow() const { return _currentRow; }
  [[nodiscard]] auto& name() const { return _name; }
private:
  // 'getColumnNumber' is used by 'Column' class constructor
  [[nodiscard]] static int getColumnNumber(const std::string& name);
  friend Column;

  using ColNames = std::map<std::string, Column>;

  void processHeaderRow(const std::string&, ColNames&);
  void verifyHeaderColumns(const ColNames&) const;

  [[nodiscard]] std::string errorMsg(const std::string& msg) const {
    auto result = msg + " - file: " + _name;
    if (_currentRow) result += ", row: " + std::to_string(_currentRow);
    return result;
  }

  std::fstream _file;
  const char _delimiter;

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
