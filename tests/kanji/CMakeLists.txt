add_executable(${TARGET} DataTest.cpp JinmeiReasonsTest.cpp KanjiDataTest.cpp
  KanjiGradesTest.cpp KanjiTest.cpp KanjiTypesTest.cpp RadicalDataTest.cpp
  UcdDataTest.cpp UcdLinkTypesTest.cpp UcdTest.cpp ../testMain.cpp)
target_link_libraries(${TARGET} PRIVATE ${LIB_PREFIX}kanji gtest gmock)
