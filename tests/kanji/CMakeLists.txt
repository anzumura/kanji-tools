add_executable(${TARGET} FileKanjiDataTest.cpp KanjiDataTest.cpp
  KanjiEnumsTest.cpp KanjiListFileTest.cpp KanjiTest.cpp MorohashiIdTest.cpp
  OfficialKanjiTest.cpp RadicalDataTest.cpp StrokesTest.cpp UcdDataTest.cpp
  UcdTest.cpp ../testMain.cpp)
target_link_libraries(${TARGET} PRIVATE ${LIB_PREFIX}kanji gtest gmock)
