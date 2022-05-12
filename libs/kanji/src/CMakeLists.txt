add_library(${TARGET} CustomFileKanji.cpp Kanji.cpp KanjiData.cpp
  KanjiListFile.cpp MorohashiId.cpp Radical.cpp RadicalData.cpp
  RealKanjiData.cpp Strokes.cpp Ucd.cpp UcdData.cpp)
target_link_libraries(${TARGET} ${LIB_PREFIX}kana)
