add_executable(${TARGET} StatsTest.cpp Utf8CountTest.cpp ../testMain.cpp)
target_link_libraries(${TARGET} PRIVATE ${LIB_PREFIX}stats gtest)
