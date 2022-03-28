add_library(${TARGET} Kana.cpp KanaConvert.cpp MBChar.cpp Table.cpp)
target_link_libraries(${TARGET} ${LIB_PREFIX}utils)
