#pragma once

#include <kt_utils/String.h>

#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <vector>

namespace kanji_tools { /// \utils_group{ColumnFile}
/// ColumnFile class for loading data from a column based text file

/// class for loading data from a delimiter (defaults to tab) separated file
/// with a header row containing the column names \utils{ColumnFile}
class ColumnFile final {
public:
  /// represents a column in a ColumnFile \utils{ColumnFile}
  ///
  /// Column instances are used to get values from each row and a Column can be
  /// be across multiple ColumnFile instances.
  class Column final {
  public:
    /// create a Column object for the given `name` (looks up `number`)
    explicit Column(const String& name);

    /// equal operator
    [[nodiscard]] bool operator==(const Column&) const;

    /// return the column name
    [[nodiscard]] auto& name() const { return _name; }

    /// return the column number (which is globally unique per column name)
    [[nodiscard]] auto number() const { return _number; }
  private:
    const String _name;
    const size_t _number;
  };

  using Columns = std::vector<Column>;    ///< list of columns for ctor
  using OptU64 = std::optional<uint64_t>; ///< optional `int64_t`
  using Path = std::filesystem::path;     ///< file path for ctor

  /// ctor creates a ColumnFile and processes the first 'header' row
  /// \param p path to the text file to be read and processed
  /// \param columns list of columns in the file (can be specified in any order)
  /// \param delim column delimiter (defaults to tab)
  /// \throw DomainError if 'p' cannot be opened (or is not a regular file) or
  ///     if a column is duplicated or not found in the header row of the file
  ColumnFile(const Path& p, const Columns& columns, char delim = '\t');

  ColumnFile(const ColumnFile&) = delete;

  /// read the next row, this method must be called before using `get` methods.
  /// \return true if a row was successfully read
  /// \throw DomainError if the next row has too few or too many columns
  bool nextRow();

  /// get the value for the given Column for the current row
  /// \throw if nextRow() hasn't been called yet or if the given Column is not
  ///     part of the ColumnFile, i.e., it wasn't passed into the ctor
  const String& get(const Column&) const;

  /// return true if the value for the given Column is empty
  [[nodiscard]] bool isEmpty(const Column&) const;

  /// get the value for the given Column and convert to `uint64_t`
  /// \throw DomainError if conversion to result type fails of if `maxValue` is
  ///     non-0 and less than the converted result
  uint64_t getU64(const Column&, uint64_t maxValue = 0) const;

  /// return `std::nullopt` if column is empty, otherwise works like getU64()
  OptU64 getOptU64(const Column&, uint64_t maxValue = 0) const;

  /// calls getU64() with the appropriate max value for numeric type `T`
  template<std::unsigned_integral T> T getUInt(const Column& c) const {
    const auto i{getU64(c, std::numeric_limits<T>::max())};
    return static_cast<T>(i);
  }

  /// calls getOptU64() with the appropriate max value for numeric type `T`
  template<std::unsigned_integral T>
  std::optional<T> getOptUInt(const Column& c) const {
    const auto i{getOptU64(c, std::numeric_limits<T>::max())};
    if (i) return static_cast<T>(*i);
    return {};
  }

  /// convenience functions for some numeric types @{
  auto getU8(const Column& c) const { return getUInt<uint8_t>(c); }
  auto getOptU8(const Column& c) const { return getOptUInt<uint8_t>(c); }
  auto getU16(const Column& c) const { return getUInt<uint16_t>(c); }
  auto getOptU16(const Column& c) const { return getOptUInt<uint16_t>(c); }
  ///@}

  /// convert 'Y' or 'T' to true, 'N', 'F' or '' to false or call 'error'
  bool getBool(const Column&) const;

  /// convert from Unicode (4 or 5 hex digit code)
  /// \throw if value is not expected size or not valid hex
  Code getChar32(const Column&) const;

  /// getChar32() overload that takes a #String instead of using the value from
  /// the given Column (the Column name is only used if there is an error)
  /// \details can be helpful when parsing a cell with comma separated values
  Code getChar32(const Column&, const String& s) const;

  /// convenience method for throwing an exception
  /// \param msg the string to use at the start of the exception 'what' value
  /// \throw DomainError that includes the file name and current row (if non-0)
  void error(const String& msg) const;

  /// error() overload that also adds info about a specific Column and value
  void error(const String& msg, const Column&, const String&) const;

  /// return the number of columns in this file
  [[nodiscard]] auto columns() const { return _rowValues.size(); }

  /// return current row number, `0` means no rows have been processed yet
  [[nodiscard]] auto currentRow() const { return _currentRow; }

  /// return the name of the file being processes
  [[nodiscard]] auto& fileName() const { return _fileName; }
private:
  /// used by Column class constructor
  [[nodiscard]] static size_t getColumnNumber(const String& name);

  [[nodiscard]] uint64_t processU64(
      const String&, const Column&, uint64_t) const;

  using ColNames = std::map<String, Column>;

  void processHeaderRow(const String&, ColNames&);
  void verifyHeaderColumns(const ColNames&) const;

  [[nodiscard]] String errorMsg(const String&) const;

  std::fstream _file;
  const char _delimiter;

  /// holds the 'last component name' of the file being processed
  const String _fileName;

  /// starts at `0` and is incremented each time 'nextRow' is called
  size_t _currentRow{};

  /// updated each time a new row is processed by 'nextRow'
  std::vector<String> _rowValues;

  /// maps a column 'number' to the position in _rowValues (starting at 0).
  /// \details This collection is populated by the ctor based on the order that
  /// the columns are found while processing the first 'header' row. A vector is
  /// used instead of a map to make lookups faster (and avoid string compares).
  /// The extra space of having a sparse collection should be minimal since
  /// columns numbers are shared if they have the same name.
  std::vector<size_t> _columnToPosition;

  /// used to globally assign unique numbers to Column instances, i.e., if the
  /// name exists then the same number is used, otherwise assign a new number.
  inline static std::map<String, size_t> _allColumns;
};

/// \end_group
} // namespace kanji_tools
