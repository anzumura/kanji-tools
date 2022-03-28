add_executable(${TARGET} MBCountTest.cpp StatsTest.cpp ../testMain.cpp)
target_include_directories(${TARGET} PRIVATE ${TESTS_INCLUDE})
target_link_libraries(${TARGET} PRIVATE ${LIB_PREFIX}stats gtest)
