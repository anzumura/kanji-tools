add_library(
  kanji_tools_stats Stats.cpp
)
target_include_directories(kanji_tools_stats PUBLIC ../../include)
target_link_libraries(kanji_tools_stats kanji_tools_kanji)
