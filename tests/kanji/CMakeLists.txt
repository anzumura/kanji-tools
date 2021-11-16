add_executable(kanjiTest KanjiTest.cpp KanjiDataTest.cpp ../testMain.cpp)
target_link_libraries(kanjiTest PRIVATE kanjiToolsKanji gtest gmock)
add_test(NAME kanji COMMAND kanjiTest)
