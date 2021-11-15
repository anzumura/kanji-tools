add_executable(
  kanji_tools_utils_test ChoiceTest.cpp FileListTest.cpp MBCharTest.cpp
  MBUtilsTest.cpp ../testMain.cpp
)
target_link_libraries(kanji_tools_utils_test PRIVATE kanji_tools_utils gtest)
add_test(NAME kanji_tools_utils_test COMMAND kanji_tools_utils_test)
