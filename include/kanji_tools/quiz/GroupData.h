#ifndef KANJI_TOOLS_QUIZ_GROUP_DATA_H
#define KANJI_TOOLS_QUIZ_GROUP_DATA_H

#include <kanji_tools/quiz/Group.h>

namespace kanji_tools {

class GroupData {
public:
  // For now, set max size for a group to '58' since this is the maximum number of entries
  // that the group quiz currently supports for entering answers, i.e., a-z, then A-Z, then
  // 6 more ascii characters following Z (before reaching 'a' again).
  enum Values { MissingTypeExamples = 12, MaxGroupSize = 58 };

  using Entry = std::shared_ptr<Group>;
  using MultiMap = std::multimap<std::string, Entry>;
  using Map = std::map<std::string, Entry>;
  using List = std::vector<Entry>;
  GroupData(DataPtr);
  GroupData(const GroupData&) = delete;

  const List& meaningGroups() const { return _meaningGroups; }
  const List& patternGroups() const { return _patternGroups; }
  const MultiMap& meaningMap() const { return _meaningMap; }
  const Map& patternMap() const { return _patternMap; }
  const Data& data() const { return *_data; }
  std::ostream& out() const { return _data->out(); }
  std::ostream& log(bool heading = false) const { return _data->log(heading); }
private:
  // 'checkInsert' will return false if 'kanji' is already in 'Map'
  bool checkInsert(const std::string& kanji, Map&, const Entry& group) const;

  // 'checkInsert' will return false if 'kanji' is already in 'MultiMap' for the given 'group'
  bool checkInsert(const std::string& kanji, MultiMap&, const Entry& group) const;

  // 'loadGroups' loads from '-groups.txt' files
  template<typename T> void loadGroup(const std::filesystem::path&, T&, List&, GroupType);

  template<typename T> void printGroups(const T&, const List&) const;

  bool fullDebug() const { return _data->fullDebug(); }

  // '_meaningMap' and '_meaningGroups' are populated from 'meaning-groups.txt'
  MultiMap _meaningMap;
  List _meaningGroups;

  // '_patternMap' and '_patternGroups' are populated from 'pattern-groups.txt.
  Map _patternMap;
  List _patternGroups;

  const DataPtr _data;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_QUIZ_GROUP_DATA_H
