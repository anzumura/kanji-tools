add_library(kanjiToolsQuiz GroupData.cpp GroupQuiz.cpp JukugoData.cpp
  ListQuiz.cpp Quiz.cpp QuizLauncher.cpp)
target_link_libraries(kanjiToolsQuiz kanjiToolsKanji)
