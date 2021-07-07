#ifndef KANJI_KANJI_GROUP_DATA
#define KANJI_KANJI_GROUP_DATA

#include <kanji/KanjiData.h>

namespace kanji {

// forward declares
class Group;
enum class GroupType;

class KanjiGroupData : public KanjiData {
public:
  using GroupEntry = std::shared_ptr<Group>;
  using GroupMap = std::map<std::string, GroupEntry>;
  using GroupList = std::vector<GroupEntry>;
  KanjiGroupData(int, const char**, std::ostream& = std::cout, std::ostream& = std::cerr);

  const GroupList& meaningGroupList() const { return _meaningGroupList; }
  const GroupList& patternGroupList() const { return _patternGroupList; }
private:
  bool checkInsert(const std::string&, GroupMap&, const GroupEntry&) const;
  // 'loadGroups' loads from '-groups.txt' files
  void loadGroup(const std::filesystem::path&, GroupMap&, GroupList&, GroupType);
  void printGroups(const GroupMap&, const GroupList&) const;

  // '_meaningGroups' and '_meaningGroupList' are populated from 'meaning-groups.txt' and
  // '_patternGroups' and '_patternGroupList' are populated from 'pattern-groups.txt. The
  // maps have an entry for each kanji to its group so currently a kanji can't be in more
  // than one group per group type.
  GroupMap _meaningGroups;
  GroupMap _patternGroups;
  GroupList _meaningGroupList;
  GroupList _patternGroupList;
};

} // namespace kanji

#endif // KANJI_KANJI_GROUP_DATA
