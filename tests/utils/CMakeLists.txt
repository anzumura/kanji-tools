add_executable(${TARGET} ArgsTest.cpp BlockRangeTest.cpp ColumnFileTest.cpp
  DataFileTest.cpp EnumArrayTest.cpp EnumArrayWithNoneTest.cpp
  EnumBitmaskTest.cpp EnumMapTest.cpp JlptLevelsTest.cpp KenteiKyusTest.cpp
  MBUtilsTest.cpp SymbolTest.cpp UnicodeBlockTest.cpp UtilsTest.cpp
  ../testMain.cpp)
target_link_libraries(${TARGET} PRIVATE ${LIB_PREFIX}utils gtest)
