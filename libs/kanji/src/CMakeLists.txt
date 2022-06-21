add_library(${TARGET} Kanji.cpp KanjiData.cpp ListFile.cpp MorohashiId.cpp
  OfficialKanji.cpp Radical.cpp RadicalData.cpp Strokes.cpp TextKanjiData.cpp
  Ucd.cpp UcdData.cpp)
target_link_libraries(${TARGET} ${LIB_PREFIX}kana)
