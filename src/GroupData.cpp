#include <kanji/Group.h>
#include <kanji/GroupData.h>
#include <kanji/MBChar.h>
#include <kanji/MBUtils.h>

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
  static const std::string wideColon("：");
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
      Group::PatternType patternType = Group::PatternType::None;
      if (name.empty()) error("Group must have a name");
      if (isAnySingleByte(name)) error("Group name must be all MB characters");
      if (type == GroupType::Pattern) {
        patternType = name.starts_with(wideColon)     ? Group::PatternType::Peer
          : name.find(wideColon) != std::string::npos ? Group::PatternType::Family
                                                      : Group::PatternType::Reading;
        // 'name' before the colon is the first member of a 'family'
        if (patternType == Group::PatternType::Family) kanjis.push_back(MBChar::getFirst(name));
      }
      if (cols[membersCol].ends_with(",")) error("members ends with ,");
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
      if (memberKanjis.size() == 1) error("group " + number + " must have more than one member");
      if (memberKanjis.size() < kanjis.size()) error("group " + number + " failed to load all members");
      if (memberKanjis.size() > MaxGroupSize)
        error("group " + number + " has more than " + std::to_string(MaxGroupSize) + "members");
      Entry group;
      if (type == GroupType::Meaning)
        group = std::make_shared<MeaningGroup>(Data::toInt(number), name, memberKanjis);
      else
        group = std::make_shared<PatternGroup>(Data::toInt(number), name, memberKanjis, patternType);
      for (const auto& i : memberKanjis)
        checkInsert(i->name(), groups, group);
      list.push_back(group);
    }
  }
}

void GroupData::printGroups(const Map& groups, const List& groupList) const {
  log() << "Loaded " << groups.size() << " kanji into " << groupList.size() << " groups\n>>> " << KanjiLegend
        << "\nName (number of entries)   Parent Member : Other Members\n";
  const int numberWidth = groupList.size() < 100 ? 2 : groupList.size() < 1000 ? 3 : 4;
  for (const auto& i : groupList) {
    out() << '[' << std::setw(numberWidth) << std::to_string(i->number()) << "]  ";
    if (i->type() == GroupType::Meaning) {
      auto len = MBChar::length(i->name());
      out() << i->name()
            << (len == 1     ? "　　"
                  : len == 2 ? "　"
                             : "")
            << " (" << std::setw(2) << std::setfill(' ') << i->members().size() << ")   :";
      for (const auto& j : i->members())
        out() << ' ' << j->qualifiedName();
    } else {
      out() << std::setw(wideSetw(i->name(), 25)) << i->name() << '(' << std::setw(2) << i->members().size() << ")   ";
      for (const auto& j : i->members())
        if (j == i->members()[0]) switch (i->patternType()) {
          case Group::PatternType::Peer: out() << "　 : " << j->qualifiedName(); break;
          case Group::PatternType::Reading: out() << j->qualifiedName(); break;
          default: out() << j->qualifiedName() << ':';
          }
        else
          out() << ' ' << j->qualifiedName();
    }
    out() << '\n';
  }
}

} // namespace kanji
