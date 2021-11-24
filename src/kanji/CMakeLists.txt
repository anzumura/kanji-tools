add_library(kanjiToolsKanji Data.cpp FileKanji.cpp Kanji.cpp KanjiData.cpp RadicalData.cpp Ucd.cpp UcdData.cpp)
target_link_libraries(kanjiToolsKanji kanjiToolsKana)
