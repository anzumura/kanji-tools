add_executable(kanji_test DataTest.cpp FileListTest.cpp KanjiTest.cpp MBCharTest.cpp testMain.cpp)
target_link_libraries(kanji_test PRIVATE kanji_lib gtest gmock)
add_test(NAME kanji_test COMMAND kanji_test)
