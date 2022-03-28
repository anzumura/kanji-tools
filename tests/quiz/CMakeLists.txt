add_executable(${TARGET} GroupDataTest.cpp QuizTest.cpp ../testMain.cpp)
target_link_libraries(${TARGET} PRIVATE ${LIB_PREFIX}quiz gtest)
