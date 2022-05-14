add_library(${TARGET} Stats.cpp Utf8Count.cpp)
target_link_libraries(${TARGET} ${LIB_PREFIX}kanji)
