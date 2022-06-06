add_executable(${TARGET} KanjiDataTest.cpp KanjiEnumsTest.cpp
  KanjiListFileTest.cpp KanjiTest.cpp MorohashiIdTest.cpp OfficialKanjiTest.cpp
  RadicalDataTest.cpp FileKanjiDataTest.cpp StrokesTest.cpp UcdDataTest.cpp
  UcdTest.cpp ../testMain.cpp)
target_link_libraries(${TARGET} PRIVATE ${LIB_PREFIX}kanji gtest gmock)
