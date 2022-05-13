add_executable(${TARGET} CustomFileKanjiTest.cpp KanjiDataTest.cpp
  KanjiEnumsTest.cpp KanjiListFileTest.cpp KanjiTest.cpp MorohashiIdTest.cpp
  RadicalDataTest.cpp RealKanjiDataTest.cpp StrokesTest.cpp UcdDataTest.cpp
  UcdTest.cpp ../testMain.cpp)
target_link_libraries(${TARGET} PRIVATE ${LIB_PREFIX}kanji gtest gmock)
