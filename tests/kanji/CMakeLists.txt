add_executable(kanjiTest JinmeiKanjiReasonsTest.cpp KanjiDataTest.cpp
  KanjiGradesTest.cpp KanjiTest.cpp KanjiTypesTest.cpp UcdLinkTypesTest.cpp
  ../testMain.cpp)
target_link_libraries(kanjiTest PRIVATE kanjiToolsKanji gtest gmock)
add_test(NAME kanji COMMAND kanjiTest)
