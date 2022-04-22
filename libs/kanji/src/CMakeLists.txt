add_library(${TARGET} CustomFileKanji.cpp Data.cpp Kanji.cpp KanjiData.cpp
  LinkedKanji.cpp MorohashiId.cpp NonLinkedKanji.cpp Radical.cpp RadicalData.cpp
  Ucd.cpp UcdData.cpp UcdFileKanji.cpp)
target_link_libraries(${TARGET} ${LIB_PREFIX}kana)
