add_executable(utilsTest ChoiceTest.cpp DisplayLengthTest.cpp FileListTest.cpp MBCharTest.cpp MBUtilsTest.cpp
   UnicodeBlockTest.cpp ../testMain.cpp)
target_link_libraries(utilsTest PRIVATE kanjiToolsUtils gtest)
add_test(NAME utils COMMAND utilsTest)
