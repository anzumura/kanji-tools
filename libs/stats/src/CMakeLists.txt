add_library(${TARGET} MBCount.cpp Stats.cpp)
target_link_libraries(${TARGET} ${LIB_PREFIX}kanji)
