#include <kanji_tools/kana/DisplaySize.h>
#include <kanji_tools/kana/Utf8Char.h>
#include <kanji_tools/quiz/GroupData.h>
#include <kanji_tools/utils/ColumnFile.h>

#include <sstream>

namespace kanji_tools {

namespace {

const String WideColon{"："};
constexpr auto MissingTypeExamples{12}, PatternGroupSetW{25}, BreakdownSetW{14};

} // namespace

const KanjiData::Path& GroupData::dataDir(const KanjiData::Path* dir) const {
  return dir ? *dir : _data->dataDir();
}

GroupData::GroupData(const KanjiDataPtr& data, const KanjiData::Path* dir)
    : _data{data} {
  loadGroup(KanjiListFile::getFile(dataDir(dir), "meaning-groups"), _meaningMap,
      _meaningGroups, GroupType::Meaning);
  loadGroup(KanjiListFile::getFile(dataDir(dir), "pattern-groups"), _patternMap,
      _patternGroups, GroupType::Pattern);
  if (_data->debug()) {
    printGroups(_meaningMap, _meaningGroups);
    printGroups(_patternMap, _patternGroups);
  }
}

void GroupData::add(const String& kanji, MultiMap& m, const GroupPtr& group) {
  m.emplace(kanji, group);
}

void GroupData::add(const String& kanji, Map& m, const GroupPtr& group) const {
  if (const auto i{m.emplace(kanji, group)}; !i.second)
    _data->printError(kanji + " from " + group->toString() + " already in " +
                      i.first->second->toString());
}

template<typename T>
void GroupData::loadGroup(
    const KanjiData::Path& file, T& groups, List& list, GroupType groupType) {
  const ColumnFile::Column numberCol{"Number"}, nameCol{"Name"},
      membersCol{"Members"};
  for (ColumnFile f(file, {numberCol, nameCol, membersCol}); f.nextRow();) {
    // get name
    auto& name{f.get(nameCol)};
    if (name.empty()) f.error("group must have a name");
    if (isAnySingleByte(name)) f.error("group name must be all MB characters");
    // get members
    auto& members{f.get(membersCol)};
    if (members.ends_with(",")) f.error("members ends with ,");
    // get patternType and kanjiNames
    auto patternType{Group::PatternType::None};
    const auto kanjiNames{getKanjiNames(name, members, groupType, patternType)};
    // get memberKanji (by looking up each name in kanjiNames)
    KanjiData::KanjiList memberKanji;
    for (auto& i : kanjiNames)
      if (const auto k{_data->findKanjiByName(i)}; k)
        memberKanji.emplace_back(k);
      else
        _data->printError("failed to find member " + i +=
                          " in group: '" + (name + "', number: ") +=
                          f.get(numberCol));
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

KanjiListFile::StringList GroupData::getKanjiNames(const String& name,
    const String& members, GroupType groupType,
    Group::PatternType& patternType) {
  KanjiListFile::StringList kanjiNames;
  if (groupType == GroupType::Pattern) {
    patternType = name.starts_with(WideColon) ? Group::PatternType::Peer
                  : name.find(WideColon) != String::npos
                      ? Group::PatternType::Family
                      : Group::PatternType::Reading;
    // 'name' before the colon is the first member of a 'family'
    if (patternType == Group::PatternType::Family)
      kanjiNames.emplace_back(Utf8Char::getFirst(name));
  }
  String member;
  for (std::stringstream ss{members}; std::getline(ss, member, ',');)
    kanjiNames.emplace_back(member);
  return kanjiNames;
}

GroupPtr GroupData::createGroup(size_t number, const String& name,
    const KanjiData::KanjiList& members, Group::PatternType patternType) {
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
    const auto len{Utf8Char::size(group.name())};
    static constexpr std::array Pad{2, 6, 4};
    out() << group.name() << std::setw(Pad[len < 3 ? len : 0]) << " ("
          << std::setw(2) << std::setfill(' ') << group.members().size()
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
  std::map<String, size_t> multipleGroups;
  String prevKey;
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
      auto& all{_data->types(i)};
      out() << std::right << std::setw(BreakdownSetW) << i << ": "
            << j->second.size() << " / " << all.size();
      printMissingFromType(all, j->second);
      out() << '\n';
    }
}

void GroupData::printMissingFromType(
    const KanjiData::KanjiList& all, StringList& found) const {
  if (const auto missing{all.size() - found.size()}; missing) {
    std::sort(found.begin(), found.end());
    out() << " (";
    for (size_t count{}; auto& i : all)
      if (!std::binary_search(found.begin(), found.end(), i->name())) {
        if (count) out() << ' ';
        out() << i->name();
        if (++count == missing || count == MissingTypeExamples) break;
      }
    out() << ')';
  }
}

} // namespace kanji_tools
