#include <gtest/gtest.h>
#include <kanji_tools/kanji/KanjiData.h>
#include <kanji_tools/quiz/GroupData.h>

namespace kanji_tools {

namespace {

class GroupDataTest : public ::testing::Test {
protected:
  // Contructs GroupData using the real data files
  GroupDataTest() : _data{std::make_shared<KanjiData>()}, _groupData{_data} {}

  const DataPtr _data;
  const GroupData _groupData;
};

} // namespace

TEST_F(GroupDataTest, SanityChecks) {
  EXPECT_FALSE(_groupData.meaningGroups().empty());
  EXPECT_FALSE(_groupData.patternGroups().empty());
  // numbers are unique and each group member is in 'groupMap'
  const auto checkNumber{[](const GroupData::List& list, const auto& groupMap) {
    std::set<size_t> uniqueNumbers;
    for (auto i : list) {
      EXPECT_TRUE(uniqueNumbers.insert(i->number()).second)
          << i->name() << " has duplicate number " << i->number();
      for (auto j : i->members())
        EXPECT_TRUE(groupMap.contains(j->name()))
            << j->name() << "from group " << i->name() << " missing from map";
    }
  }};
  checkNumber(_groupData.meaningGroups(), _groupData.meaningMap());
  checkNumber(_groupData.patternGroups(), _groupData.patternMap());
}

} // namespace kanji_tools
