#include <kanji_tools/quiz/GroupData.h>
#include <kanji_tools/utils/ColumnFile.h>
#include <kanji_tools/utils/DisplayLength.h>
#include <kanji_tools/utils/MBChar.h>

#include <sstream>

namespace kanji_tools {

namespace fs = std::filesystem;

namespace {

const fs::path MeaningGroupFile = "meaning-groups.txt";
const fs::path PatternGroupFile = "pattern-groups.txt";

const std::string WideColon("：");

} // namespace

GroupData::GroupData(DataPtr data) : _data(data) {
  loadGroup(DataFile::getFile(_data->dataDir(), MeaningGroupFile), _meaningMap, _meaningGroups, GroupType::Meaning);
  loadGroup(DataFile::getFile(_data->dataDir(), PatternGroupFile), _patternMap, _patternGroups, GroupType::Pattern);
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

bool GroupData::checkInsert(const std::string& name, MultiMap& groups, const Entry& group) const {
  auto i = groups.equal_range(name);
  for (auto j = i.first; j != i.second; ++j)
    if (j->second->number() == group->number()) {
      _data->printError(name + " from Group " + std::to_string(group->number()) + " already in same group");
      return false;
    }
  groups.insert(std::make_pair(name, group));
  return true;
}

template<typename T>
void GroupData::loadGroup(const std::filesystem::path& file, T& groups, List& list, GroupType groupType) {
  ColumnFile::Column numberCol("Number"), nameCol("Name"), membersCol("Members");
  for (ColumnFile f(file, {numberCol, nameCol, membersCol}); f.nextRow();) {
    const std::string &name(f.get(nameCol)), members(f.get(membersCol));
    if (name.empty()) f.error("group must have a name");
    if (isAnySingleByte(name)) f.error("group name must be all MB characters");
    if (members.ends_with(",")) f.error("members ends with ,");

    DataFile::List kanjiNames;
    Group::PatternType patternType = Group::PatternType::None;
    if (groupType == GroupType::Pattern) {
      patternType = name.starts_with(WideColon)     ? Group::PatternType::Peer
        : name.find(WideColon) != std::string::npos ? Group::PatternType::Family
                                                    : Group::PatternType::Reading;
      // 'name' before the colon is the first member of a 'family'
      if (patternType == Group::PatternType::Family) kanjiNames.push_back(MBChar::getFirst(name));
    }
    std::string member;
    for (std::stringstream ss(members); std::getline(ss, member, ',');)
      kanjiNames.emplace_back(member);
    Data::List memberKanji;
    for (const auto& i : kanjiNames)
      if (auto k = _data->findKanjiByName(i); k)
        memberKanji.push_back(*k);
      else
        _data->printError("failed to find member " + i + " in group: '" + name + "', number: " + f.get(numberCol));
    if (memberKanji.empty()) f.error("group has no valid members");
    if (memberKanji.size() == 1) f.error("group must have more than one member");
    if (memberKanji.size() < kanjiNames.size()) f.error("group failed to load all members");
    if (memberKanji.size() > MaxGroupSize) f.error("group has more than " + std::to_string(MaxGroupSize) + " members");

    Entry group = createGroup(f.getInt(numberCol), name, memberKanji, patternType);
    for (const auto& i : memberKanji)
      checkInsert(i->name(), groups, group);
    list.push_back(group);
  }
}

GroupData::Entry GroupData::createGroup(int number, const std::string& name, const Data::List& members,
                                        Group::PatternType patternType) const {
  if (patternType == Group::PatternType::None) return std::make_shared<MeaningGroup>(number, name, members);
  return std::make_shared<PatternGroup>(number, name, members, patternType);
}

template<typename T> void GroupData::printGroups(const T& groups, const List& groupList) const {
  log() << "Loaded " << groups.size() << " kanji into " << groupList.size() << " groups\n";
  if (fullDebug()) log() << Kanji::Legend << "\nName (number of entries)   Parent Member : Other Members\n";
  TypeMap types;
  StringSet uniqueNames;
  for (const int numberWidth = groupList.size() < 100 ? 2 : groupList.size() < 1000 ? 3 : 4; auto& i : groupList) {
    if (fullDebug()) out() << '[' << std::setw(numberWidth) << std::to_string(i->number()) << "]  ";
    if (i->type() == GroupType::Meaning)
      printMeaningGroup(*i, types, uniqueNames);
    else
      printPatternGroup(*i, types);
    if (fullDebug()) out() << '\n';
  }
  if (!uniqueNames.empty() && uniqueNames.size() < groups.size()) printUniqueNames(groups, uniqueNames);
  printTypeBreakdown(types);
}

void GroupData::printMeaningGroup(const Group& group, TypeMap& types, StringSet& uniqueNames) const {
  if (fullDebug()) {
    auto len = MBChar::length(group.name());
    out() << group.name()
          << (len == 1     ? "　　"
                : len == 2 ? "　"
                           : "")
          << " (" << std::setw(2) << std::setfill(' ') << group.members().size() << ")   :";
  }
  for (const auto& i : group.members()) {
    if (fullDebug()) out() << ' ' << i->qualifiedName();
    // the same kanji can be in more than one meaning group so check uniqueness to avoid overcounting
    if (uniqueNames.insert(i->name()).second) types[i->type()].push_back(i->name());
  }
}

void GroupData::printPatternGroup(const Group& group, TypeMap& types) const {
  if (fullDebug())
    out() << std::setw(wideSetw(group.name(), 25)) << group.name() << '(' << std::setw(2) << group.members().size()
          << ")   ";
  for (const auto& i : group.members()) {
    types[i->type()].push_back(i->name());
    if (fullDebug()) {
      if (i == group.members()[0]) switch (group.patternType()) {
        case Group::PatternType::Peer: out() << "　 : " << i->qualifiedName(); break;
        case Group::PatternType::Reading: out() << i->qualifiedName(); break;
        default: out() << i->qualifiedName() << ':';
        }
      else
        out() << ' ' << i->qualifiedName();
    }
  }
}

template<typename T> void GroupData::printUniqueNames(const T& groups, const StringSet& uniqueNames) const {
  std::map<std::string, int> multipleGroups;
  std::string prevKey;
  int maxBelongsTo = 0; // will hold the maximum number of groups a kanji belongs to
  for (auto i = groups.begin(); i != groups.end(); ++i)
    if (i->first == prevKey) {
      if (int belongsTo = ++multipleGroups[i->first]; belongsTo > maxBelongsTo) maxBelongsTo = belongsTo;
    } else
      prevKey = i->first;
  log() << "Unique kanji: " << uniqueNames.size() << " (once " << uniqueNames.size() - multipleGroups.size()
        << ", multi " << multipleGroups.size() << ")\n";
  for (int i = 1; i <= maxBelongsTo; ++i) {
    out() << "  Kanji in " << i + 1 << " groups:";
    for (auto j = multipleGroups.begin(); j != multipleGroups.end(); ++j)
      if (j->second == i) out() << ' ' << j->first;
    out() << '\n';
  }
}

void GroupData::printTypeBreakdown(TypeMap& types) const {
  out() << "Type Breakdown (showing up to " << MissingTypeExamples << " missing examples per type)\n";
  for (auto i : AllKanjiTypes)
    if (auto j = types.find(i); j != types.end()) {
      const Data::List& list = _data->typeList(i);
      out() << std::right << std::setw(14) << i << ": " << j->second.size() << " / " << list.size();
      if (int missing = list.size() - j->second.size(); missing < 0)
        out() << " -- ERROR: negative missing values"; // shouldn't be possible
      else if (missing > 0) {
        std::sort(j->second.begin(), j->second.end());
        out() << " (";
        for (int count = 0; auto& k : list)
          if (!std::binary_search(j->second.begin(), j->second.end(), k->name())) {
            if (count) out() << ' ';
            out() << k->name();
            if (++count == missing || count == MissingTypeExamples) break;
          }
        out() << ')';
      }
      out() << '\n';
    }
}

} // namespace kanji_tools
