add_executable(${TARGET} ChoiceTest.cpp ColumnFileTest.cpp DataFileTest.cpp
  DisplaySizeTest.cpp EnumArrayTest.cpp EnumArrayWithNoneTest.cpp
  EnumBitmaskTest.cpp EnumMapTest.cpp JlptLevelsTest.cpp KenteiKyusTest.cpp
  MBUtilsTest.cpp UnicodeBlockTest.cpp ../testMain.cpp)
target_include_directories(${TARGET} PRIVATE ${TESTS_INCLUDE})
target_link_libraries(${TARGET} PRIVATE ${LIB_PREFIX}utils gtest)
