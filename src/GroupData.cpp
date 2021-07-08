#include <kanji/Group.h>
#include <kanji/GroupData.h>
#include <kanji/MBChar.h>

#include <fstream>
#include <sstream>

namespace kanji {

namespace fs = std::filesystem;

namespace {

const fs::path MeaningGroupFile = "meaning-groups.txt";
const fs::path PatternGroupFile = "pattern-groups.txt";

} // namespace

GroupData::GroupData(DataPtr data) : _data(data) {
  loadGroup(FileList::getFile(_data->dataDir(), MeaningGroupFile), _meaningMap, _meaningGroups, GroupType::Meaning);
  loadGroup(FileList::getFile(_data->dataDir(), PatternGroupFile), _patternMap, _patternGroups, GroupType::Pattern);
  if (_data->debug()) {
    printGroups(_meaningMap, _meaningGroups);
    printGroups(_patternMap, _patternGroups);
  }
}

bool GroupData::checkInsert(const std::string& name, Map& groups, const Entry& group) const {
  auto i = groups.insert(std::make_pair(name, group));
  if (!i.second)
    _data->printError(name + " from Group " + std::to_string(group->number()) + " already in group " +
                      i.first->second->toString());
  return i.second;
}

void GroupData::loadGroup(const std::filesystem::path& file, Map& groups, List& list, GroupType type) {
  int lineNumber = 1, numberCol = -1, nameCol = -1, membersCol = -1;
  auto error = [&lineNumber, &file](const std::string& s, bool printLine = true) {
    Data::usage(s + (printLine ? " - line: " + std::to_string(lineNumber) : "") + ", file: " + file.string());
  };
  auto setCol = [&file, &error](int& col, int pos) {
    if (col != -1) error("column " + std::to_string(pos) + " has duplicate name", false);
    col = pos;
  };
  std::ifstream f(file);
  std::array<std::string, 3> cols;
  for (std::string line; std::getline(f, line); ++lineNumber) {
    int pos = 0;
    std::stringstream ss(line);
    if (numberCol == -1) {
      for (std::string token; std::getline(ss, token, '\t'); ++pos)
        if (token == "Number")
          setCol(numberCol, pos);
        else if (token == "Name")
          setCol(nameCol, pos);
        else if (token == "Members")
          setCol(membersCol, pos);
        else
          error("unrecognized column '" + token + "'", false);
      if (pos != cols.size()) error("not enough columns", false);
    } else {
      for (std::string token; std::getline(ss, token, '\t'); ++pos) {
        if (pos == cols.size()) error("too many columns");
        cols[pos] = token;
      }
      if (pos != cols.size()) error("not enough columns");
      std::string number(cols[numberCol]), name(cols[nameCol]), token;
      FileList::List kanjis;
      const bool peers = name.empty();
      if (type == GroupType::Meaning) {
        if (peers) error("Meaning group must have a name");
      } else if (!peers) // if populated, 'name colum' is the first member of a Pattern group
        kanjis.emplace_back(name);
      for (std::stringstream members(cols[membersCol]); std::getline(members, token, ',');)
        kanjis.emplace_back(token);
      Data::List memberKanjis;
      for (const auto& i : kanjis) {
        const auto memberKanji = _data->findKanji(i);
        if (memberKanji.has_value())
          memberKanjis.push_back(*memberKanji);
        else
          _data->printError("failed to find member " + i + " in group " + number);
      }
      if (memberKanjis.empty()) error("group " + number + " has no valid members");
      Entry group;
      if (type == GroupType::Meaning)
        group = std::make_shared<MeaningGroup>(FileListKanji::toInt(number), name, memberKanjis);
      else
        group = std::make_shared<PatternGroup>(FileListKanji::toInt(number), memberKanjis, peers);
      for (const auto& i : memberKanjis)
        checkInsert(i->name(), groups, group);
      list.push_back(group);
    }
  }
}

void GroupData::printGroups(const Map& groups, const List& groupList) const {
  log() << "Loaded " << groups.size() << " kanji into " << groupList.size() << " groups\n>>> " << KanjiLegend << ":\n";
  for (const auto& i : groupList) {
    if (i->type() == GroupType::Meaning) {
      auto len = MBChar::length(i->name());
      out() << '[' << i->name()
            << (len == 1     ? "　　"
                  : len == 2 ? "　"
                             : "")
            << ' ' << std::setw(2) << std::setfill(' ') << i->members().size() << "] :";
      for (const auto& j : i->members())
        out() << ' ' << j->qualifiedName();
    } else {
      out() << '[' << std::setw(3) << std::setfill('0') << i->number() << "] ";
      for (const auto& j : i->members())
        if (j == i->members()[0]) {
          if (i->peers())
            out() << "　 : " << j->qualifiedName();
          else
            out() << j->qualifiedName() << ':';
        } else
          out() << ' ' << j->qualifiedName();
    }
    out() << '\n';
  }
}

} // namespace kanji
