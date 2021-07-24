add_executable(
  kanji_test ChoiceTest.cpp FileListTest.cpp FileStatsTest.cpp GroupDataTest.cpp
  KanaTest.cpp KanaConvertTest.cpp KanjiTest.cpp KanjiDataTest.cpp MBCharTest.cpp
  MBUtilsTest.cpp QuizTest.cpp testMain.cpp
)
target_link_libraries(kanji_test PRIVATE kanji_lib gtest gmock)
add_test(NAME kanji_test COMMAND kanji_test)
