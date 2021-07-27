#ifndef KANJI_TABLE_H
#define KANJI_TABLE_H

#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace kanji {

// 'Table' is for printing out tables of data to a console or stream. It will adjust cells to
// have same width as the widest cell per column. It also takes into account 'Wide Characters'
// having twice the width of normal characters. The output aligns properly on a terminal using
/// a 'monospace' font, but can be slightly off in web pages or an IDE (like VS Code) depending
// on the font family.
// For example the following 'VS Code Font Family' fonts are 'fixed-width', but wide characters
// are not exactly double the size of normal characters:
// - Nice for coding: Menlo, Monaco, Lucida Console, Lucida Sans Typewriter, Consolas
// - Harder to read: PT Mono, Courier, Courier New, Andale Mono, Monospace
// The following fonts display wide characters as exactly double the width of normal, but are
// harder to read for normal width characters:
// - SimHei, Osaka-Mono, MS Mincho, MS Gothic, MingLiU, PCMyungjo, SimSun-ExtB
class Table {
public:
  using Row = std::vector<std::string>;
  using Rows = std::vector<Row>;

  Table(const Row& title = {}, bool countInFirstColumn = false)
    : _title(title), _countInFirstColumn(countInFirstColumn) {}

  // 'add' will add 'row' to the table. If 'startNewSection' is true then a horizontal border will be
  // printed before printing the row. Row can have less columns than other rows (or even be empty) -
  // in this case the missing columns are assumed to be empty cells.
  void add(const Row& row = {}, bool startNewSection = false);

  // 'print' will print the table to 'os' with a border at the top and bottom.
  void print(std::ostream& os = std::cout) const;
private:
  using Widths = std::vector<size_t>;
  void print(std::ostream&, const Widths&, const Row&, char fill = ' ', char delim = '|') const;
  // 'border' prints a horizontal border row
  void border(std::ostream& os, const Widths& w) const { print(os, w, {}, '-', '+'); }

  const Row _title;
  const bool _countInFirstColumn;
  Rows _rows;
  std::set<int> _sections;
};

} // namespace kanji

#endif // KANJI_TABLE_H
