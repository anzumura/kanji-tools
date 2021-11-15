add_executable(
  kanji_tools_kanji_test KanjiTest.cpp KanjiDataTest.cpp ../testMain.cpp
)
target_link_libraries(kanji_tools_kanji_test PRIVATE kanji_tools_kanji gtest gmock)
add_test(NAME kanji_tools_kanji_test COMMAND kanji_tools_kanji_test)
