add_library(${TARGET} Converter.cpp Kana.cpp KanaConvert.cpp MBChar.cpp
  Table.cpp)
target_link_libraries(${TARGET} ${LIB_PREFIX}utils)
