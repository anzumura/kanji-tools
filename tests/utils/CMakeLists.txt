add_executable(utilsTest ChoiceTest.cpp FileListTest.cpp MBCharTest.cpp MBUtilsTest.cpp ../testMain.cpp)
target_link_libraries(utilsTest PRIVATE kanjiToolsUtils gtest)
add_test(NAME utils COMMAND utilsTest)
