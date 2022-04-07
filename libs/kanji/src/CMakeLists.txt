add_library(${TARGET} CustomFileKanji.cpp Data.cpp Kanji.cpp KanjiData.cpp
  LinkedKanji.cpp RadicalData.cpp Ucd.cpp UcdData.cpp)
target_link_libraries(${TARGET} ${LIB_PREFIX}kana)
