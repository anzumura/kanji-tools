#include <kanji_tools/kana/DisplaySize.h>
#include <kanji_tools/kana/MBChar.h>
#include <kanji_tools/quiz/GroupData.h>
#include <kanji_tools/utils/ColumnFile.h>
#include <kanji_tools/utils/Utils.h>

#include <sstream>

namespace kanji_tools {

namespace {

const std::string WideColon{"："};
// LCOV_EXCL_START: covered
constexpr auto MissingTypeExamples{12}, PatternGroupSetW{25}, BreakdownSetW{14};
// LCOV_EXCL_STOP

} // namespace

const Data::Path& GroupData::dataDir(const Data::Path* dir) const {
  return dir ? *dir : _data->dataDir();
}

GroupData::GroupData(DataPtr data, const Data::Path* dir) : _data{data} {
  loadGroup(DataFile::getFile(dataDir(dir), "meaning-groups"), _meaningMap,
      _meaningGroups, GroupType::Meaning);
  loadGroup(DataFile::getFile(dataDir(dir), "pattern-groups"), _patternMap,
      _patternGroups, GroupType::Pattern);
  if (_data->debug()) {
    printGroups(_meaningMap, _meaningGroups);
    printGroups(_patternMap, _patternGroups);
  }
}

void GroupData::add(
    const std::string& name, Map& groups, const GroupPtr& group) const {
  const auto i{groups.emplace(name, group)};
  if (!i.second)
    _data->printError(name + " from " + group->toString() + " already in " +
                      i.first->second->toString());
}

void GroupData::add(
    const std::string& name, MultiMap& groups, const GroupPtr& group) const {
  groups.emplace(name, group);
}

template<typename T>
void GroupData::loadGroup(
    const Data::Path& file, T& groups, List& list, GroupType groupType) {
  const ColumnFile::Column numberCol{"Number"}, nameCol{"Name"},
      membersCol{"Members"};
  for (ColumnFile f(file, {numberCol, nameCol, membersCol}); f.nextRow();) {
    auto& name{f.get(nameCol)};
    auto& members{f.get(membersCol)};
    if (name.empty()) f.error("group must have a name");
    if (isAnySingleByte(name)) f.error("group name must be all MB characters");
    if (members.ends_with(",")) f.error("members ends with ,");

    DataFile::StringList kanjiNames;
    auto patternType{Group::PatternType::None};
    if (groupType == GroupType::Pattern) {
      patternType = name.starts_with(WideColon) ? Group::PatternType::Peer
                    : name.find(WideColon) != std::string::npos
                        ? Group::PatternType::Family
                        : Group::PatternType::Reading;
      // 'name' before the colon is the first member of a 'family'
      if (patternType == Group::PatternType::Family)
        kanjiNames.emplace_back(MBChar::getFirst(name));
    }
    std::string member;
    for (std::stringstream ss{members}; std::getline(ss, member, ',');)
      kanjiNames.emplace_back(member);
    Data::KanjiList memberKanji;
    for (auto& i : kanjiNames)
      if (const auto k{_data->findKanjiByName(i)}; k)
        memberKanji.emplace_back(k);
      else
        _data->printError("failed to find member " + i + " in group: '" + name +
                          "', number: " + f.get(numberCol));
    if (memberKanji.size() < kanjiNames.size())
      f.error("group failed to load all members");
    try {
      auto group{
          createGroup(f.getULong(numberCol), name, memberKanji, patternType)};
      for (auto& i : memberKanji) add(i->name(), groups, group);
      list.emplace_back(group);
    } catch (const std::exception& e) {
      f.error(e.what());
    }
  }
}

GroupPtr GroupData::createGroup(size_t number, const std::string& name,
    const Data::KanjiList& members, Group::PatternType patternType) const {
  if (patternType == Group::PatternType::None)
    return std::make_shared<MeaningGroup>(number, name, members);
  return std::make_shared<PatternGroup>(number, name, members, patternType);
}

template<typename T>
void GroupData::printGroups(const T& groups, const List& groupList) const {
  log() << "Loaded " << groups.size() << " kanji into " << groupList.size()
        << " groups\n";
  if (fullDebug())
    log() << Kanji::Legend
          << "\nName (number of entries)   Parent Member : Other Members\n";
  TypeMap types;
  StringSet uniqueNames;
  for (const auto numberWidth{groupList.size() < 100    ? 2
                              : groupList.size() < 1000 ? 3
                                                        : 4};
       auto& i : groupList) {
    if (fullDebug())
      out() << '[' << std::setw(numberWidth) << std::to_string(i->number())
            << "]  ";
    if (i->type() == GroupType::Meaning)
      printMeaningGroup(*i, types, uniqueNames);
    else
      printPatternGroup(*i, types);
    if (fullDebug()) out() << '\n';
  }
  if (!uniqueNames.empty() && uniqueNames.size() < groups.size())
    printUniqueNames(groups, uniqueNames);
  printTypeBreakdown(types);
}

void GroupData::printMeaningGroup(
    const Group& group, TypeMap& types, StringSet& uniqueNames) const {
  if (fullDebug()) {
    const auto len{MBChar::size(group.name())};
    out() << group.name()
          << (len == 1      ? "　　"
                 : len == 2 ? "　"
                            : "")
          << " (" << std::setw(2) << std::setfill(' ') << group.members().size()
          << ")   :";
  }
  for (auto& i : group.members()) {
    if (fullDebug()) out() << ' ' << i->qualifiedName();
    // the same kanji can be in more than one meaning group so check uniqueness
    // to avoid overcounting
    if (uniqueNames.insert(i->name()).second)
      types[i->type()].emplace_back(i->name());
  }
}

void GroupData::printPatternGroup(const Group& group, TypeMap& types) const {
  if (fullDebug())
    out() << std::setw(wideSetw(group.name(), PatternGroupSetW)) << group.name()
          << '(' << std::setw(2) << group.members().size() << ")   ";
  for (auto& i : group.members()) {
    types[i->type()].emplace_back(i->name());
    if (fullDebug()) {
      if (i == group.members()[0]) switch (group.patternType()) {
        case Group::PatternType::Peer:
          out() << "　 : " << i->qualifiedName();
          break;
        case Group::PatternType::Reading: out() << i->qualifiedName(); break;
        case Group::PatternType::Family:
        case Group::PatternType::None: out() << i->qualifiedName() << ':';
        }
      else
        out() << ' ' << i->qualifiedName();
    }
  }
}

template<typename T>
void GroupData::printUniqueNames(
    const T& groups, const StringSet& uniqueNames) const {
  std::map<std::string, size_t> multipleGroups;
  std::string prevKey;
  size_t maxBelongsTo{}; // the maximum number of groups a kanji belongs to
  for (auto i{groups.begin()}; i != groups.end(); ++i)
    if (i->first == prevKey) {
      if (const auto belongsTo{++multipleGroups[i->first]};
          belongsTo > maxBelongsTo)
        maxBelongsTo = belongsTo;
    } else
      prevKey = i->first;
  log() << "Unique kanji: " << uniqueNames.size() << " (once "
        << uniqueNames.size() - multipleGroups.size() << ", multi "
        << multipleGroups.size() << ")\n";
  for (size_t i{1}; i <= maxBelongsTo; ++i) {
    out() << "  Kanji in " << i + 1 << " groups:";
    for (auto j{multipleGroups.begin()}; j != multipleGroups.end(); ++j)
      if (j->second == i) out() << ' ' << j->first;
    out() << '\n';
  }
}

void GroupData::printTypeBreakdown(TypeMap& types) const {
  out() << "Type Breakdown (showing up to " << MissingTypeExamples
        << " missing examples per type)\n";
  for (auto i : AllKanjiTypes)
    if (auto j{types.find(i)}; j != types.end()) {
      auto& list{_data->types(i)};
      out() << std::right << std::setw(BreakdownSetW) << i << ": "
            << j->second.size() << " / " << list.size();
      if (const auto missing{list.size() - j->second.size()}; missing) {
        std::sort(j->second.begin(), j->second.end());
        out() << " (";
        for (size_t count{}; auto& k : list)
          if (!std::binary_search(
                  j->second.begin(), j->second.end(), k->name())) {
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
