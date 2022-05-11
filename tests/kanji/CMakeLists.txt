add_executable(${TARGET} CustomFileKanjiTest.cpp DataTest.cpp KanjiDataTest.cpp
  KanjiGradesTest.cpp KanjiTest.cpp MorohashiIdTest.cpp RadicalDataTest.cpp
  StrokesTest.cpp UcdDataTest.cpp UcdTest.cpp ../testMain.cpp)
target_link_libraries(${TARGET} PRIVATE ${LIB_PREFIX}kanji gtest gmock)
