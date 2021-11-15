add_library(
  kanji_tools_kanji Data.cpp Kanji.cpp KanjiData.cpp RadicalData.cpp UcdData.cpp
)
target_include_directories(kanji_tools_kanji PUBLIC ../../include)
target_link_libraries(kanji_tools_kanji kanji_tools_kana)
