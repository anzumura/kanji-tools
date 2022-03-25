add_executable(statsTest MBCountTest.cpp StatsTest.cpp ../testMain.cpp)
target_include_directories(statsTest PRIVATE ${TESTS_INCLUDE})
target_link_libraries(statsTest PRIVATE kanjiToolsStats gtest)
add_test(NAME stats COMMAND statsTest)
