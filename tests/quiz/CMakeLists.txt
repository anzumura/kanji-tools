add_executable(
  kanji_tools_quiz_test GroupDataTest.cpp QuizTest.cpp ../testMain.cpp
)
target_link_libraries(kanji_tools_quiz_test PRIVATE kanji_tools_quiz gtest)
add_test(NAME kanji_tools_quiz_test COMMAND kanji_tools_quiz_test)
