#pragma once

#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace kanji_tools {

// 'ColumnFile' is a helper class for loading data from a delimiter (defaults to
// tab) separated text file with a header row (containing the column names).
class ColumnFile {
public:
  static inline const std::string EmptyString;

  // 'Column' has a name that must match a column header in the file being
  // processed. The set of columns for a 'ColumnFile' are passed into its
  // constructor and the same column instances are used to get values from each
  // row. A 'Column' can be used across multiple 'ColumnFile' instances.
  class Column {
  public:
    Column(const std::string& name)
        : _name(name), _number(ColumnFile::getColumnNumber(name)) {}

    [[nodiscard]] auto operator==(const Column& rhs) const {
      return _number == rhs._number;
    }

    [[nodiscard]] auto& name() const { return _name; }
    [[nodiscard]] auto number() const { return _number; }
  private:
    const std::string _name;
    const size_t _number; // globally unique number per column based on '_name'
  };

  using Columns = std::vector<Column>;
  using OptSize = std::optional<size_t>;

  // 'ColumnFile' will throw an exception if 'p' cannot be opened (or is not a
  // regular file) or if the list of 'columns' doesn't match the first row of
  // the file. Note, the columns in the file can be in a different order than
  // 'columns', but the names must all be found.
  ColumnFile(const std::filesystem::path& p, const Columns& columns,
             char delimiter = '\t');

  ColumnFile(const ColumnFile&) = delete;
  // operator= is not generated since there are const members

  // 'nextRow' must be called before using 'get'. An exception is thrown if the
  // next row has too few or too many columns. 'nextRow' returns 'false' when
  // there are no more rows in the file.
  bool nextRow();

  // 'get' returns the value for the given column for the current row. An
  // exception is thrown if 'nextRow' hasn't been called yet or if the given
  // column was not passed in to the constructor.
  const std::string& get(const Column&) const;

  [[nodiscard]] auto isEmpty(const Column& column) const {
    return get(column).empty();
  }

  // convert to 'size_t' or call 'error'
  size_t getSize(const Column&) const;

  // return std::nullopt if column is empty or return 'getSize'
  OptSize getOptSize(const Column&) const;

  // convert 'Y' or 'T' to true, 'N', 'F' or '' to false or call 'error'
  bool getBool(const Column&) const;

  // overload for a specific 's' (can be used for columns with multiple values)
  char32_t getWChar(const Column&, const ::std::string& s) const;

  // convert from Unicode (4 or 5 hex code) or call 'error'
  auto getWChar(const Column& c) const { return getWChar(c, get(c)); }

  // throw a 'domain_error' exception with 'what' string made from 'msg' plus
  // '_name'. '_currentRow' is also added if it's not zero.
  void error(const std::string& msg) const {
    throw std::domain_error(errorMsg(msg));
  }

  // overload for reporting a problem with a specific value for a column
  void error(const std::string& msg, const Column& c,
             const std::string& s) const {
    throw std::domain_error(errorMsg(msg) + ", column: '" + c.name() +
                            "', value: '" + s + "'");
  }

  [[nodiscard]] auto columns() const { return _rowValues.size(); }
  [[nodiscard]] auto currentRow() const { return _currentRow; }
  [[nodiscard]] auto& name() const { return _name; }
private:
  // 'getColumnNumber' is used by 'Column' class constructor
  [[nodiscard]] static size_t getColumnNumber(const std::string& name);
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
  size_t _currentRow{};

  // '_rowValues' is updated each time a new row is processed by 'nextRow'
  std::vector<std::string> _rowValues;

  // '_columnToPosition' maps a column 'number' to the position in _rowValues
  // (starting at 0). This collection is populated by the constructor based on
  // the order that the columns are found while processing the first 'header'
  // row. A vector is used instead of a map to make lookups faster (and avoid
  // string compares) and the extra space of having a sparse collection should
  // be minimal since columns numbers are shared if they have the same name.
  std::vector<size_t> _columnToPosition;

  // '_allColumns' is used to globally assign unique numbers to 'Column'
  // instances, i.e., if the column name exists then the same number is used,
  // otherwise a new number is assigned.
  inline static std::map<std::string, size_t> _allColumns;
};

} // namespace kanji_tools
