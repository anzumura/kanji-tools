#pragma once

#include <kanji_tools/utils/String.h>

#include <iostream>
#include <set>
#include <vector>

namespace kanji_tools { /// \kana_group{Table}
/// Table class for printing data in tabular form \kana{Table}

/// class for printing data as a plain-text or Markdown table \kana{Table}
///
/// For plain-text print(), cells are adjusted to have same width as the widest
/// cell per column. This also takes into account 'Wide Characters' having twice
/// the width of normal characters. The output aligns properly on a terminal
/// using a 'monospace' font, but can be slightly off in web pages or an IDE
/// (like VS Code) depending on the font. For web pages, use printMarkdown().
///
/// \note The following 'VS Code Font Family' fonts are 'fixed-width', but wide
/// characters are not exactly double the size of normal characters:
/// \li Nicer: Menlo, Monaco, Lucida Console, Lucida Sans Typewriter, Consolas
/// \li Harder to read: PT Mono, Courier, Courier New, Andale Mono, Monospace
///
/// The following fonts display wide characters as exactly double the width of
/// normal, but are harder to read for normal width characters:
/// \li SimHei, Osaka-Mono, MS Mincho, MS Gothic, MingLiU, PCMyungjo,
/// SimSun-ExtB
class Table {
public:
  using Row = std::vector<String>; ///< a row in the Table

  /// create a Table with an optional header row
  /// \param title header row (can be empty), should include an entry for the
  ///     automatically generated count column (if `countInFirstColumn` is true)
  /// \param countInFirstColumn if true then a cell with the current row number
  ///     is added to the beginning of each row
  explicit Table(const Row& title = {}, bool countInFirstColumn = false)
      : _title{title}, _countInFirstColumn{countInFirstColumn} {}

  Table(const Table&) = delete; ///< deleted copy ctor

  /// add a row
  /// \param row the row to be added, can have less columns than other rows (or
  ///     even empty) - in this case the missing columns are assumed to be empty
  /// \param startNewSection if true then a horizontal border will be printed
  ///     before printing this row (or the row will be in bold for Markdown)
  void add(const Row& row = {}, bool startNewSection = false);

  ///  print to `os` with a border at the top and bottom
  void print(std::ostream& os = std::cout) const;

  /// print a Markdown table to `os` suitable for putting into a .md file
  ///
  /// Since Markdown tables need a header row, empty values will be used if
  /// #_title is empty. Also, 'sections' are supported by marking contents of
  /// the row as bold (instead of using borders).
  ///
  /// \note Values containing pipes (|) characters mess up rendering so they are
  /// escaped, but no other accommodations are done for Markdown (like checking
  /// for other HTML type sequences).
  void printMarkdown(std::ostream& os = std::cout) const;
private:
  using Widths = std::vector<size_t>; ///< list of column widths
  using Rows = std::vector<Row>;      ///< list or rows

  static void printRow(std::ostream&, const Widths&, const Row&,
      char fill = ' ', char delim = '|');

  /// print a horizontal border row
  static void border(std::ostream&, const Widths&);

  /// used by 'printMarkdown...' functions
  enum class RowType { Normal, Header, Section };

  /// called from printMarkdown() to print a row
  static void printMarkdownRow(
      std::ostream&, size_t maxColumns, const Row&, RowType = RowType::Normal);

  const Row _title;
  const bool _countInFirstColumn;
  Rows _rows;
  std::set<size_t> _sections;
};

/// \end_group
} // namespace kanji_tools
