add_executable(kanaTest KanaConvertTest.cpp KanaTest.cpp MBCharTest.cpp
  TableTest.cpp ../testMain.cpp)
target_include_directories(kanaTest PRIVATE ${TESTS_INCLUDE})
target_link_libraries(kanaTest PRIVATE kanjiToolsKana gtest)
add_test(NAME kana COMMAND kanaTest)
