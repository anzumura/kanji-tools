add_library(${TARGET} CustomFileKanji.cpp Data.cpp Kanji.cpp KanjiData.cpp
  KanjiListFile.cpp MorohashiId.cpp Radical.cpp RadicalData.cpp Strokes.cpp
  Ucd.cpp UcdData.cpp)
target_link_libraries(${TARGET} ${LIB_PREFIX}kana)
