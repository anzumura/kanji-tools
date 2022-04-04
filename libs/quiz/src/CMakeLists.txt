add_library(${TARGET} Group.cpp GroupData.cpp GroupQuiz.cpp Jukugo.cpp
  JukugoData.cpp ListQuiz.cpp Quiz.cpp QuizLauncher.cpp)
target_link_libraries(${TARGET} ${LIB_PREFIX}kanji)
