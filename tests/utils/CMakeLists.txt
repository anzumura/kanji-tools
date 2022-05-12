add_executable(${TARGET} ArgsTest.cpp BitmaskTest.cpp BlockRangeTest.cpp
  ColumnFileTest.cpp EnumListTest.cpp EnumListWithNoneTest.cpp EnumMapTest.cpp
  MBUtilsTest.cpp StringTest.cpp SymbolTest.cpp UnicodeBlockTest.cpp
  ../testMain.cpp)
target_link_libraries(${TARGET} PRIVATE ${LIB_PREFIX}utils gtest)
