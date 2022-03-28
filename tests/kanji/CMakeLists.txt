add_executable(${TARGET} JinmeiKanjiReasonsTest.cpp KanjiDataTest.cpp
  KanjiGradesTest.cpp KanjiTest.cpp KanjiTypesTest.cpp UcdLinkTypesTest.cpp
  ../testMain.cpp)
target_include_directories(${TARGET} PRIVATE ${TESTS_INCLUDE})
target_link_libraries(${TARGET} PRIVATE ${LIB_PREFIX}kanji gtest gmock)
