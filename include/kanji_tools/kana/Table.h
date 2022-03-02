#pragma once

#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace kanji_tools {

// 'Table' is for printing out tables of data to a console or stream. It will
// adjust cells to have same width as the widest cell per column. It also takes
// into account 'Wide Characters' having twice the width of normal characters.
// The output aligns properly on a terminal using a 'monospace' font, but can be
// slightly off in web pages or an IDE (like VS Code) depending on the font.
// For example the following 'VS Code Font Family' fonts are 'fixed-width', but
// wide characters are not exactly double the size of normal characters:
// - Nicer: Menlo, Monaco, Lucida Console, Lucida Sans Typewriter, Consolas
// - Harder to read: PT Mono, Courier, Courier New, Andale Mono, Monospace
// The following fonts display wide characters as exactly double the width of
// normal, but are harder to read for normal width characters:
// - SimHei, Osaka-Mono, MS Mincho, MS Gothic, MingLiU, PCMyungjo, SimSun-ExtB
class Table {
public:
  using Row = std::vector<std::string>;
  using Rows = std::vector<Row>;

  Table(const Row& title = {}, bool countInFirstColumn = false)
      : _title(title), _countInFirstColumn(countInFirstColumn) {}

  Table(const Table&) = delete;
  // operator= is not generated since there are const members

  // add 'row' to the table. If 'startNewSection' is true then a horizontal
  // border will be printed before printing the row. Row can have less columns
  // than other rows (or even be empty) - in this case the missing columns are
  // assumed to be empty cells.
  void add(const Row& row = {}, bool startNewSection = false);

  // print the table to 'os' with a border at the top and bottom.
  void print(std::ostream& os = std::cout) const;

  // 'printMarkdown' will print out the table suitable for putting into a
  // README.md file. Since Markdown tables need a header row, empty values will
  // be used if _title is empty. Also, 'sections' are supported by marking
  // contents of the row as bold (instead of using borders). Note, a value
  // containing a pipe (|) character will mess up rendering the table so these
  // characters are escaped, but no other accommodations are done for markdown
  // (like checking for other html type sequences).
  void printMarkdown(std::ostream& os = std::cout) const;
private:
  using Widths = std::vector<size_t>;

  void print(std::ostream&, const Widths&, const Row&, char fill = ' ',
             char delim = '|') const;

  // 'border' prints a horizontal border row
  void border(std::ostream& os, const Widths& w) const {
    print(os, w, {}, '-', '+');
  }

  const Row _title;
  const bool _countInFirstColumn;
  Rows _rows;
  std::set<size_t> _sections;
};

} // namespace kanji_tools
