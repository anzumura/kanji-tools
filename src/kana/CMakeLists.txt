add_library(kanjiToolsKana Kana.cpp KanaConvert.cpp MBChar.cpp Table.cpp)
target_include_directories(kanjiToolsKana PUBLIC include)
target_link_libraries(kanjiToolsKana kanjiToolsUtils)
