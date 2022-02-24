add_executable(kanjiTest JinmeiKanjiReasonsTest.cpp KanjiTest.cpp
  KanjiDataTest.cpp KanjiGradesTest.cpp KanjiTypesTest.cpp UcdLinkTypesTest.cpp
  ../testMain.cpp)
target_link_libraries(kanjiTest PRIVATE kanjiToolsKanji gtest gmock)
add_test(NAME kanji COMMAND kanjiTest)
