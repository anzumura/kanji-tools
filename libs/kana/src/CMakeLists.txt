add_library(${TARGET} Choice.cpp Converter.cpp DisplaySize.cpp Kana.cpp
  KanaConvert.cpp MBChar.cpp Table.cpp)
target_link_libraries(${TARGET} ${LIB_PREFIX}utils)
