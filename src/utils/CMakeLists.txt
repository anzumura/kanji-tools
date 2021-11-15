add_library(
  kanji_tools_utils Choice.cpp FileList.cpp MBChar.cpp Table.cpp Ucd.cpp
)
target_include_directories(kanji_tools_utils PUBLIC ../../include)
