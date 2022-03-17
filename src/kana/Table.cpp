#include <kanji_tools/kana/Table.h>
#include <kanji_tools/utils/DisplaySize.h>

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
    if (!_title.empty()) print(os, widths, _title);
    for (size_t i{}; i < _rows.size(); ++i) {
      if (_sections.contains(i)) border(os, widths);
      print(os, widths, _rows[i]);
    }
    border(os, widths);
  }
}

void Table::printMarkdown(std::ostream& os) const {
  size_t maxColumns{_title.size()};
  for (auto& i : _rows) maxColumns = std::max(maxColumns, i.size());
  const auto pRow{[&](const Row& r, bool header = false, bool section = false) {
    for (size_t i{}; i < maxColumns; ++i) {
      os << "| ";
      if (header && r.empty()) os << "---";
      if (i < r.size()) {
        std::string out;
        out.reserve(r[i].size());
        for (const auto c : r[i])
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
  }};
  if (maxColumns) {
    // Markdown needs a header row followed by a row for formatting (---, :-:,
    // etc.) so print _title even if it's empty (which will just make and empty
    // set of headers).
    pRow(_title);
    pRow({}, true);
    for (size_t i{}; i < _rows.size(); ++i)
      pRow(_rows[i], false, _sections.contains(i));
  }
}

void Table::print(std::ostream& os, const Widths& widths, const Row& row,
    char fill, char delim) const {
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
      cell(widths[i], EmptyString);
  os << delim << '\n';
}

} // namespace kanji_tools
