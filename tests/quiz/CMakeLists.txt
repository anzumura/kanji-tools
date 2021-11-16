add_executable(quizTest GroupDataTest.cpp QuizTest.cpp ../testMain.cpp)
target_link_libraries(quizTest PRIVATE kanjiToolsQuiz gtest)
add_test(NAME quiz COMMAND quizTest)
