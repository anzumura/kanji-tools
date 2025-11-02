add_executable(kanaConvert convertMain.cpp)
target_link_libraries(kanaConvert PRIVATE ${LIB_PREFIX}kana)

add_executable(kanjiFormat formatMain.cpp)
target_link_libraries(kanjiFormat PRIVATE ${LIB_PREFIX}utils)

add_executable(kanjiQuiz quizMain.cpp)
target_link_libraries(kanjiQuiz PRIVATE ${LIB_PREFIX}quiz)

add_executable(kanjiStats statsMain.cpp)
target_link_libraries(kanjiStats PRIVATE ${LIB_PREFIX}stats)
