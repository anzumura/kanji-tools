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
  using ULong = uint64_t;

  // 'Column' has a name that must match a column header in the file being
  // processed. The set of columns for a 'ColumnFile' are passed into its ctor
  // and the same column instances are used to get values rows. A 'Column' can
  // be used across multiple 'ColumnFile' instances.
  class Column {
  public:
    Column(const std::string& name);

    [[nodiscard]] bool operator==(const Column&) const;

    [[nodiscard]] auto& name() const { return _name; }
    [[nodiscard]] auto number() const { return _number; }
  private:
    const std::string _name;
    const size_t _number; // globally unique number per column based on '_name'
  };

  using Columns = std::vector<Column>;
  using OptULong = std::optional<ULong>;

  // 'ColumnFile' will throw an exception if 'p' cannot be opened (or is not a
  // regular file) or if the list of columns doesn't match the first row of the
  // file. Note, columns in the file can be in a different order than the list
  // provided to this ctor, but the names must all be found.
  ColumnFile(const std::filesystem::path& p, const Columns&, char delim = '\t');

  ColumnFile(const ColumnFile&) = delete;

  // 'nextRow' must be called before using 'get'. An exception is thrown if the
  // next row has too few or too many columns. 'nextRow' returns 'false' when
  // there are no more rows in the file.
  bool nextRow();

  // 'get' returns the value for the given column for the current row. An
  // exception is thrown if 'nextRow' hasn't been called yet or if the given
  // column was not passed in to the constructor.
  const std::string& get(const Column&) const;

  [[nodiscard]] bool isEmpty(const Column&) const;

  // convert to 'ULong' or call 'error' (if maxValue is non-zero and
  // value exceeds it then call 'error')
  ULong getULong(const Column&, ULong maxValue = 0) const;

  // return std::nullopt if column is empty or call 'processULong'
  OptULong getOptULong(const Column&, ULong maxValue = 0) const;

  // getUInt takes a numeric type T (like u_int16_t) and then calls getLong with
  // the appropriate max value
  template<std::unsigned_integral T> T getUInt(const Column& c) const {
    const auto i{getULong(c, std::numeric_limits<T>::max())};
    return static_cast<T>(i);
  }

  // getOptUInt takes a numeric type T and then calls getOptLong with the
  // appropriate max value
  template<std::unsigned_integral T>
  std::optional<T> getOptUInt(const Column& column) const {
    const auto i{getOptULong(column, std::numeric_limits<T>::max())};
    if (i) return static_cast<T>(*i);
    return {};
  }

  // convenience functions for some numberic types
  auto getU8(const Column& c) const { return getUInt<u_int8_t>(c); }
  auto getOptU8(const Column& c) const { return getOptUInt<u_int8_t>(c); }
  auto getU16(const Column& c) const { return getUInt<u_int16_t>(c); }
  auto getOptU16(const Column& c) const { return getOptUInt<u_int16_t>(c); }

  // convert 'Y' or 'T' to true, 'N', 'F' or '' to false or call 'error'
  bool getBool(const Column&) const;

  // overload for a specific 's' (can be used for columns with multiple values)
  char32_t getChar32(const Column&, const ::std::string& s) const;

  // convert from Unicode (4 or 5 hex code) or call 'error'
  char32_t getChar32(const Column&) const;

  // throw a 'domain_error' exception with 'what' string made from 'msg' plus
  // '_fileName'. '_currentRow' is also added if it's not zero.
  void error(const std::string& msg) const;

  // overload for reporting a problem with a specific value 's' for a column
  void error(const std::string& msg, const Column&, const std::string& s) const;

  [[nodiscard]] auto columns() const { return _rowValues.size(); }
  [[nodiscard]] auto currentRow() const { return _currentRow; }
  [[nodiscard]] auto& fileName() const { return _fileName; }
private:
  // 'getColumnNumber' is used by 'Column' class constructor
  [[nodiscard]] static size_t getColumnNumber(const std::string& name);

  [[nodiscard]] ULong processULong(
      const std::string&, const Column&, ULong maxValue) const;

  using ColNames = std::map<std::string, Column>;

  void processHeaderRow(const std::string&, ColNames&);
  void verifyHeaderColumns(const ColNames&) const;

  [[nodiscard]] std::string errorMsg(const std::string&) const;

  std::fstream _file;
  const char _delimiter;

  // '_fileName' holds the 'last component name' of the file being processed
  const std::string _fileName;

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
