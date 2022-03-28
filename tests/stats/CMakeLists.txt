add_executable(${TARGET} MBCountTest.cpp StatsTest.cpp ../testMain.cpp)
target_link_libraries(${TARGET} PRIVATE ${LIB_PREFIX}stats gtest)
