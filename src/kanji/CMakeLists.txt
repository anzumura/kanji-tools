add_library(kanjiToolsKanji Data.cpp Kanji.cpp KanjiData.cpp RadicalData.cpp UcdData.cpp)
target_link_libraries(kanjiToolsKanji kanjiToolsKana)
