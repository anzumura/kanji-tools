#pragma once

#include <kanji_tools/quiz/Group.h>

namespace kanji_tools {

// 'GroupData' creates 'Group' instances from the '-groups.txt' files. 'Group'
// class prevents duplicates, but this class also prevent a Kanji from being in
// multiple Pattern Groups (see comments at the bottom of this file for details)
class GroupData {
public:
  using MultiMap = std::multimap<String, GroupPtr>;
  using Map = std::map<String, GroupPtr>;
  using List = std::vector<GroupPtr>;

  // if 'dir' is provided it will be used intead of 'data->dataDir()' when
  // looking for group files (to help with testing)
  explicit GroupData(const DataPtr&, const Data::Path* dir = {});

  GroupData(const GroupData&) = delete;
  GroupData& operator=(const GroupData&) = delete;

  [[nodiscard]] auto& meaningGroups() const { return _meaningGroups; }
  [[nodiscard]] auto& patternGroups() const { return _patternGroups; }
  [[nodiscard]] auto& meaningMap() const { return _meaningMap; }
  [[nodiscard]] auto& patternMap() const { return _patternMap; }
  [[nodiscard]] auto& data() const { return *_data; }
  [[nodiscard]] auto& out() const { return _data->out(); }
  [[nodiscard]] auto& log(bool heading = false) const {
    return _data->log(heading);
  }
private:
  const Data::Path& dataDir(const Data::Path*) const;

  // add 'kanji'->'group' mapping (no error is logged 'MultiMap' override)
  static void add(const String& kanji, MultiMap&, const GroupPtr& group);

  // add 'kanji'->'group' mapping or log an error if it's already been added
  void add(const String& kanji, Map&, const GroupPtr& group) const;

  template<typename T> void loadGroup(const Data::Path&, T&, List&, GroupType);

  static KanjiListFile::StringList getKanjiNames(const String& name,
      const String& members, GroupType, Group::PatternType&);

  [[nodiscard]] static GroupPtr createGroup(size_t number, const String& name,
      const Data::KanjiList&, Group::PatternType);

  template<typename T> void printGroups(const T&, const List&) const;

  using StringList = std::vector<String>;
  using TypeMap = std::map<KanjiTypes, StringList>;
  using StringSet = std::set<String>;

  void printMeaningGroup(const Group&, TypeMap&, StringSet&) const;
  void printPatternGroup(const Group&, TypeMap&) const;
  template<typename T> void printUniqueNames(const T&, const StringSet&) const;
  void printTypeBreakdown(TypeMap&) const;
  void printMissingFromType(const Data::KanjiList&, StringList&) const;

  [[nodiscard]] auto fullDebug() const { return _data->fullDebug(); }

  // '_meaningMap' and '_meaningGroups' are populated from
  // 'meaning-groups.txt'
  MultiMap _meaningMap;
  List _meaningGroups;

  // '_patternMap' and '_patternGroups' are populated from
  // 'pattern-groups.txt.
  Map _patternMap;
  List _patternGroups;

  const DataPtr _data;
};

using GroupDataPtr = std::shared_ptr<const GroupData>;

// Not allowing a Kanji to be in multiple Pattern Groups can be ambiguous for
// some more complex Kanji so in these cases, the pattern that best fits
// related pronunciation was chosen (as well as preferring grouping by
// 'non-radical') when creating the 'pattern-groups.txt' file. This
// restriction doesn't apply to Meaning groups since choosing only one meaning
// for some (even very common) Kanji would make other groups seem incomplete,
// e.g., if '金' was only in the '色' group then the '時間：曜日' group would
// be missing a day.

} // namespace kanji_tools
