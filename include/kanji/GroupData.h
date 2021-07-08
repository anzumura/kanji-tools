#ifndef KANJI_GROUP_DATA
#define KANJI_GROUP_DATA

#include <kanji/Data.h>

namespace kanji {

// forward declares
class Group;
enum class GroupType;

class GroupData {
public:
  using Entry = std::shared_ptr<Group>;
  using Map = std::map<std::string, Entry>;
  using List = std::vector<Entry>;
  GroupData(DataPtr);
  GroupData(const GroupData&) = delete;

  const List& meaningGroups() const { return _meaningGroups; }
  const List& patternGroups() const { return _patternGroups; }
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

} // namespace kanji

#endif // KANJI_GROUP_DATA
