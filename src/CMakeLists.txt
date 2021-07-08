add_library(
  kanji_lib Choice.cpp Data.cpp FileList.cpp FileStats.cpp GroupData.cpp
  Kanji.cpp KanjiData.cpp Quiz.cpp MBChar.cpp
)
target_include_directories(kanji_lib PUBLIC ../include)
