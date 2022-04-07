add_executable(${TARGET} ArgsTest.cpp BlockRangeTest.cpp ChoiceTest.cpp
  ColumnFileTest.cpp DataFileTest.cpp DisplaySizeTest.cpp EnumArrayTest.cpp
  EnumArrayWithNoneTest.cpp EnumBitmaskTest.cpp EnumMapTest.cpp
  JlptLevelsTest.cpp KenteiKyusTest.cpp MBUtilsTest.cpp UnicodeBlockTest.cpp
  UtilsTest.cpp ../testMain.cpp)
target_link_libraries(${TARGET} PRIVATE ${LIB_PREFIX}utils gtest)
