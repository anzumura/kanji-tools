add_executable(kanaTest KanaTest.cpp KanaConvertTest.cpp MBCharTest.cpp ../testMain.cpp)
target_link_libraries(kanaTest PRIVATE kanjiToolsKana gtest)
add_test(NAME kana COMMAND kanaTest)
