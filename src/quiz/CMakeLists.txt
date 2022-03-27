add_library(kanjiToolsQuiz GroupData.cpp GroupQuiz.cpp JukugoData.cpp
  ListQuiz.cpp Quiz.cpp QuizLauncher.cpp)
target_include_directories(kanjiToolsQuiz PUBLIC include)
target_link_libraries(kanjiToolsQuiz kanjiToolsKanji)
