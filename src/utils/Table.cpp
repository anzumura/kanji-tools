#include <kanji_tools/utils/DisplayLength.h>
#include <kanji_tools/utils/Table.h>

#include <iomanip>

namespace kanji_tools {

void Table::add(const Row& row, bool startNewSection) {
  if (startNewSection) _sections.insert(_rows.size());
  if (_countInFirstColumn) {
    Row r(row);
    r.insert(r.begin(), std::to_string(_rows.size() + 1));
    _rows.emplace_back(r);
  } else
    _rows.push_back(row);
}

void Table::print(std::ostream& os) const {
  Widths widths;
  for (auto& i : _title) widths.push_back(displayLength(i));
  for (auto& row : _rows) {
    for (size_t colNum = 0; colNum < row.size(); ++colNum) {
      if (colNum < widths.size())
        widths[colNum] = std::max(widths[colNum], displayLength(row[colNum]));
      else
        widths.push_back(displayLength(row[colNum]));
    }
  }
  if (!widths.empty()) {
    border(os, widths);
    if (!_title.empty()) print(os, widths, _title);
    for (size_t i = 0; i < _rows.size(); ++i) {
      if (_sections.contains(i)) border(os, widths);
      print(os, widths, _rows[i]);
    }
    border(os, widths);
  }
}

void Table::printMarkdown(std::ostream& os) const {
  size_t maxColumns = _title.size();
  for (auto& i : _rows) maxColumns = std::max(maxColumns, i.size());
  auto printRow = [&os, maxColumns](const Row& r, bool header = false, bool section = false) {
    for (size_t i = 0; i < maxColumns; ++i) {
      os << "| ";
      if (header && r.empty()) os << "---";
      if (i < r.size()) {
        std::string out;
        out.reserve(r[i].size());
        for (char c : r[i])
          if (c == '|')
            out += "\\|";
          else
            out += c;
        if (out.empty() || !section)
          os << out;
        else
          os << "**" << out << "**";
      }
      os << ' ';
    }
    os << "|\n";
  };
  if (maxColumns) {
    // Markdown needs a header row followed by a row for formatting (---, :-:, etc.) so
    // print _title even if it's empty (which will just make and empty set of headers).
    printRow(_title);
    printRow({}, true);
    for (size_t i = 0; i < _rows.size(); ++i) printRow(_rows[i], false, _sections.contains(i));
  }
}

void Table::print(std::ostream& os, const Widths& w, const Row& r, char fill, char delim) const {
  static const std::string empty;
  auto cell = [&os, delim, fill](int w, const auto& s) { os << delim << fill << std::setw(w + 1) << s; };
  os << std::left << std::setfill(fill);
  for (size_t i = 0; i < w.size(); ++i)
    if (i < r.size())
      // if string is all narrow chars then nothing will be added, but if there are wide
      // chars then we need to add the difference to get 'setw' to work properly.
      cell(w[i] + (r[i].length() - displayLength(r[i])), r[i]);
    else
      cell(w[i], empty);
  os << delim << '\n';
}

} // namespace kanji_tools
