add_executable(${TARGET} KanjiDataTest.cpp KanjiEnumsTest.cpp KanjiTest.cpp
  ListFileTest.cpp MorohashiIdTest.cpp OfficialKanjiTest.cpp RadicalDataTest.cpp
  StrokesTest.cpp TextKanjiDataTest.cpp UcdDataTest.cpp UcdTest.cpp
  ../testMain.cpp)
target_link_libraries(${TARGET} PRIVATE ${LIB_PREFIX}kanji gtest gmock)
