add_library(
  kanji_tools_kana Kana.cpp KanaConvert.cpp
)
target_include_directories(kanji_tools_kana PUBLIC ../../include)
target_link_libraries(kanji_tools_kana kanji_tools_utils)
