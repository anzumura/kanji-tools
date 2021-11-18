add_library(kanjiToolsKanji Data.cpp Kanji.cpp KanjiData.cpp OtherKanji.cpp RadicalData.cpp UcdData.cpp)
target_link_libraries(kanjiToolsKanji kanjiToolsKana)
