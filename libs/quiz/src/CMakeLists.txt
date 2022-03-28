add_library(${TARGET} GroupData.cpp GroupQuiz.cpp JukugoData.cpp ListQuiz.cpp
  Quiz.cpp QuizLauncher.cpp)
target_link_libraries(${TARGET} ${LIB_PREFIX}kanji)
