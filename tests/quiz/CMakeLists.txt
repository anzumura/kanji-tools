add_executable(${TARGET} GroupDataTest.cpp GroupQuizTest.cpp GroupTest.cpp
  JukugoDataTest.cpp JukugoTest.cpp ListQuizTest.cpp QuizLauncherTest.cpp
  QuizTest.cpp ../testMain.cpp)
target_link_libraries(${TARGET} PRIVATE ${LIB_PREFIX}quiz gtest)
