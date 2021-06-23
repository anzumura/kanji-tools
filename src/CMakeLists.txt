add_library(kanji_lib Data.cpp FileList.cpp Kanji.cpp MBChar.cpp)
target_include_directories(kanji_lib PUBLIC ../include)
