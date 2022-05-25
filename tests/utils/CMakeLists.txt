add_executable(${TARGET} ArgsTest.cpp BitmaskTest.cpp BlockRangeTest.cpp
  ColumnFileTest.cpp EnumListTest.cpp EnumListWithNoneTest.cpp EnumMapTest.cpp
  ExceptionTest.cpp StringTest.cpp SymbolTest.cpp UnicodeBlockTest.cpp
  Utf8Test.cpp ../testMain.cpp)
target_link_libraries(${TARGET} PRIVATE ${LIB_PREFIX}utils gtest)
