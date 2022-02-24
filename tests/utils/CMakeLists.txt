add_executable(utilsTest ChoiceTest.cpp ColumnFileTest.cpp DisplayLengthTest.cpp
  DataFileTest.cpp EnumArrayTest.cpp EnumArrayWithNoneTest.cpp
  EnumBitmaskTest.cpp JlptLevelsTest.cpp KenteiKyusTest.cpp MBUtilsTest.cpp
  UnicodeBlockTest.cpp ../testMain.cpp)
target_link_libraries(utilsTest PRIVATE kanjiToolsUtils gtest)
add_test(NAME utils COMMAND utilsTest)
