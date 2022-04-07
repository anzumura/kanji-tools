add_executable(${TARGET} DataTest.cpp JinmeiKanjiReasonsTest.cpp
  KanjiDataTest.cpp KanjiGradesTest.cpp KanjiTest.cpp KanjiTypesTest.cpp
  RadicalDataTest.cpp UcdTest.cpp UcdLinkTypesTest.cpp ../testMain.cpp)
target_link_libraries(${TARGET} PRIVATE ${LIB_PREFIX}kanji gtest gmock)
