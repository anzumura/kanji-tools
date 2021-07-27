#ifndef KANJI_TABLE_H
#define KANJI_TABLE_H

#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace kanji {

class Table {
public:
  using Row = std::vector<std::string>;
  using Rows = std::vector<Row>;

  Table(Row title = Row(), bool countInFirstColumn = false) : _title(title), _countInFirstColumn(countInFirstColumn) {}

  // 'add' will add 'row' to the table. If 'startNewSection' is true then a horizontal border will be
  // printed before printing the row. Row can have less columns than other rows (or even be empty) -
  // in this case the missing columns are assumed to be empty cells.
  void add(const Row& row = Row(), bool startNewSection = false);

  // 'print' will print the table to 'os' with a border at the top and bottom.
  void print(std::ostream& os) const;
private:
  using Widths = std::vector<size_t>;
  void print(std::ostream&, const Widths&, const Row&, char fill = ' ', char delim = '|') const;
  // 'border' prints a horizontal border row
  void border(std::ostream& os, const Widths& w) const { print(os, w, Row(), '-', '+'); }

  const Row _title;
  const bool _countInFirstColumn;
  Rows _rows;
  std::set<int> _sections;
};

} // namespace kanji

#endif // KANJI_TABLE_H
