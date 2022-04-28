#include <kanji_tools/kana/DisplaySize.h>
#include <kanji_tools/kana/Table.h>

#include <iomanip>

namespace kanji_tools {

void Table::add(const Row& row, bool startNewSection) {
  if (startNewSection) _sections.insert(_rows.size());
  if (_countInFirstColumn) {
    Row r{row};
    r.insert(r.begin(), std::to_string(_rows.size() + 1));
    _rows.emplace_back(std::move(r));
  } else
    _rows.emplace_back(row);
}

void Table::print(std::ostream& os) const {
  Widths widths;
  for (auto& i : _title) widths.push_back(displaySize(i));
  for (auto& row : _rows)
    for (size_t i{}; auto& col : row) {
      if (const auto w{displaySize(col)}; i < widths.size()) {
        if (widths[i] < w) widths[i] = w;
      } else
        widths.push_back(w);
      ++i;
    }
  if (!widths.empty()) {
    border(os, widths);
    if (!_title.empty()) printRow(os, widths, _title);
    for (size_t i{}; i < _rows.size(); ++i) {
      if (_sections.contains(i)) border(os, widths);
      printRow(os, widths, _rows[i]);
    }
    border(os, widths);
  }
}

void Table::printMarkdown(std::ostream& os) const {
  size_t maxColumns{_title.size()};
  for (auto& i : _rows) maxColumns = std::max(maxColumns, i.size());
  if (maxColumns) {
    // Markdown needs a header row followed by a row for formatting (---, :-:,
    // etc.) so print _title even if it's empty (which will just make and empty
    // set of headers).
    printMarkdownRow(os, maxColumns, _title);
    printMarkdownRow(os, maxColumns, {}, RowType::Header);
    for (size_t i{}; i < _rows.size(); ++i)
      printMarkdownRow(os, maxColumns, _rows[i],
          _sections.contains(i) ? RowType::Section : RowType::Normal);
  }
}

void Table::printRow(std::ostream& os, const Widths& widths, const Row& row,
    char fill, char delim) {
  const auto cell{[&os, delim, fill](size_t w, const auto& s) {
    os << delim << fill << std::setw(static_cast<int>(w) + 1) << s;
  }};
  os << std::left << std::setfill(fill);
  for (size_t i{}; i < widths.size(); ++i)
    if (i < row.size())
      // if string is all narrow then nothing will be added, but if there are
      // wide chars then need to add difference to get 'setw' to work properly
      cell(widths[i] + (row[i].size() - displaySize(row[i])), row[i]);
    else
      cell(widths[i], "");
  os << delim << '\n';
}

void Table::border(std::ostream& os, const Widths& w) {
  printRow(os, w, {}, '-', '+');
}

void Table::printMarkdownRow(
    std::ostream& os, size_t maxColumns, const Row& row, RowType rowType) {
  for (size_t i{}; i < maxColumns; ++i) {
    os << "| ";
    if (rowType == RowType::Header && row.empty()) os << "---";
    if (i < row.size()) {
      std::string out;
      out.reserve(row[i].size());
      for (const auto c : row[i])
        if (c == '|')
          out += "\\|";
        else
          out += c;
      if (out.empty() || rowType != RowType::Section)
        os << out;
      else
        os << "**" << out << "**";
    }
    os << ' ';
  }
  os << "|\n";
}

} // namespace kanji_tools
