#pragma once

#include <kanji_tools/quiz/Group.h>

namespace kanji_tools {

class GroupData {
public:
  // For now, set max size for a group to '58' since this is the maximum number
  // of entries that the group quiz currently supports for entering answers,
  // i.e., a-z, then A-Z, then 6 more ascii characters following Z (before
  // reaching 'a' again).
  enum Values { MissingTypeExamples = 12, MaxGroupSize = 58 };

  using Entry = std::shared_ptr<Group>;
  using MultiMap = std::multimap<std::string, Entry>;
  using Map = std::map<std::string, Entry>;
  using List = std::vector<Entry>;

  GroupData(DataPtr);

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
  // return false if 'kanji' is already in 'Map'
  bool checkInsert(const std::string& kanji, Map&, const Entry& group) const;

  // return false if 'kanji' is already in 'MultiMap' for the given 'group'
  bool checkInsert(
      const std::string& kanji, MultiMap&, const Entry& group) const;

  template<typename T>
  void loadGroup(const std::filesystem::path&, T&, List&, GroupType);

  [[nodiscard]] Entry createGroup(size_t number, const std::string& name,
      const Data::List& members, Group::PatternType) const;

  template<typename T> void printGroups(const T&, const List&) const;

  using TypeMap = std::map<KanjiTypes, std::vector<std::string>>;
  using StringSet = std::set<std::string>;

  void printMeaningGroup(const Group&, TypeMap&, StringSet&) const;
  void printPatternGroup(const Group&, TypeMap&) const;
  template<typename T> void printUniqueNames(const T&, const StringSet&) const;
  void printTypeBreakdown(TypeMap&) const;

  [[nodiscard]] auto fullDebug() const { return _data->fullDebug(); }

  // '_meaningMap' and '_meaningGroups' are populated from 'meaning-groups.txt'
  MultiMap _meaningMap;
  List _meaningGroups;

  // '_patternMap' and '_patternGroups' are populated from 'pattern-groups.txt.
  Map _patternMap;
  List _patternGroups;

  const DataPtr _data;
};

using GroupDataPtr = std::shared_ptr<const GroupData>;

} // namespace kanji_tools
