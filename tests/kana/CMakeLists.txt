add_executable(
  kanji_tools_kana_test KanaTest.cpp KanaConvertTest.cpp ../testMain.cpp
)
target_link_libraries(kanji_tools_kana_test PRIVATE kanji_tools_kana gtest)
add_test(NAME kanji_tools_kana_test COMMAND kanji_tools_kana_test)
