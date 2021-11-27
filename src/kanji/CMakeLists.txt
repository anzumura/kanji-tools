add_library(kanjiToolsKanji CustomFileKanji.cpp Data.cpp Kanji.cpp KanjiData.cpp RadicalData.cpp Ucd.cpp UcdData.cpp)
target_link_libraries(kanjiToolsKanji kanjiToolsKana)
