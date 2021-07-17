#include <gtest/gtest.h>

#include <kanji/GroupData.h>
#include <kanji/KanjiData.h>

namespace kanji {

class GroupDataTest : public ::testing::Test {
protected:
  static const char** argv() {
    static const char* arg0 = "testMain";
    static const char* arg1 = "-data";
    static const char* arg2 = "../../data";
    static const char* args[] = {arg0, arg1, arg2};
    return args;
  }
  // Contructs GroupData using the real data files
  GroupDataTest()
    : _data(std::make_shared<KanjiData>(3, argv())), _groupData(_data) {}

  const DataPtr _data;
  const GroupData _groupData;
};

TEST_F(GroupDataTest, GroupsLoaded) {
  EXPECT_FALSE(_groupData.meaningGroups().empty());
  EXPECT_FALSE(_groupData.patternGroups().empty());
}

} // namespace kanji
