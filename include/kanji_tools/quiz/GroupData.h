#ifndef KANJI_TOOLS_QUIZ_GROUP_DATA_H
#define KANJI_TOOLS_QUIZ_GROUP_DATA_H

#include <kanji_tools/kanji/Data.h>

namespace kanji_tools {

// forward declares
class Group;
enum class GroupType;

class GroupData {
public:
  // For now, set max size for a group to '58' since this is the maximum number of entries
  // that the group quiz currently supports for entering answers, i.e., a-z, then A-Z, then
  // 6 more ascii characters following Z (before reaching 'a' again).
  enum Values { MissingTypeExamples = 12, MaxGroupSize = 58 };

  using Entry = std::shared_ptr<Group>;
  using Map = std::map<std::string, Entry>;
  using List = std::vector<Entry>;
  GroupData(DataPtr);
  GroupData(const GroupData&) = delete;

  const List& meaningGroups() const { return _meaningGroups; }
  const List& patternGroups() const { return _patternGroups; }
  const Map& meaningMap() const { return _meaningMap; }
  const Map& patternMap() const { return _patternMap; }
  const Data& data() const { return *_data; }
  std::ostream& out() const { return _data->out(); }
  std::ostream& log(bool heading = false) const { return _data->log(heading); }
private:
  bool checkInsert(const std::string&, Map&, const Entry&) const;
  // 'loadGroups' loads from '-groups.txt' files
  void loadGroup(const std::filesystem::path&, Map&, List&, GroupType);
  void printGroups(const Map&, const List&) const;

  // '_meaningMap' and '_meaningGroups' are populated from 'meaning-groups.txt' and
  // '_patternMap' and '_patternGroups' are populated from 'pattern-groups.txt. The
  // maps have an entry for each kanji to its group so currently a kanji can't be in
  // more than one group per group type.
  Map _meaningMap;
  Map _patternMap;
  List _meaningGroups;
  List _patternGroups;
  const DataPtr _data;
};

} // namespace kanji_tools

#endif // KANJI_TOOLS_QUIZ_GROUP_DATA_H
