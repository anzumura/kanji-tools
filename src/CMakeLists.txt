add_library(
  kanji_lib Choice.cpp Data.cpp FileList.cpp Kanji.cpp KanjiCount.cpp
  KanjiData.cpp KanjiGroupData.cpp KanjiQuiz.cpp MBChar.cpp
)
target_include_directories(kanji_lib PUBLIC ../include)
