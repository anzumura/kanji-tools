add_library(
  kanji_tools_quiz GroupData.cpp Quiz.cpp
)
target_include_directories(kanji_tools_quiz PUBLIC ../../include)
target_link_libraries(kanji_tools_quiz kanji_tools_kanji)
