add_library(kanjiToolsKana Kana.cpp KanaConvert.cpp MBChar.cpp Table.cpp)
target_link_libraries(kanjiToolsKana kanjiToolsUtils)
