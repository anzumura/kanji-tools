add_executable(kanaTest KanaConvertTest.cpp KanaTest.cpp MBCharTest.cpp
  TableTest.cpp ../testMain.cpp)
target_link_libraries(kanaTest PRIVATE kanjiToolsKana gtest)
add_test(NAME kana COMMAND kanaTest)
