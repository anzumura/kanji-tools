add_executable(utilsTest ChoiceTest.cpp ColumnFileTest.cpp DataFileTest.cpp
  DisplaySizeTest.cpp EnumArrayTest.cpp EnumArrayWithNoneTest.cpp
  EnumBitmaskTest.cpp EnumMapTest.cpp JlptLevelsTest.cpp KenteiKyusTest.cpp
  MBUtilsTest.cpp UnicodeBlockTest.cpp ../testMain.cpp)
target_include_directories(utilsTest PRIVATE ${TESTS_INCLUDE})
target_link_libraries(utilsTest PRIVATE kanjiToolsUtils gtest)
add_test(NAME utils COMMAND utilsTest)
