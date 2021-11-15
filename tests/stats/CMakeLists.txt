add_executable(
  kanji_tools_stats_test StatsTest.cpp ../testMain.cpp
)
target_link_libraries(kanji_tools_stats_test PRIVATE kanji_tools_stats gtest)
add_test(NAME kanji_tools_stats_test COMMAND kanji_tools_stats_test)
