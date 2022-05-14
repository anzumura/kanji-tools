add_executable(${TARGET} ChoiceTest.cpp ConverterTest.cpp DisplaySizeTest.cpp
  KanaConvertTest.cpp KanaTest.cpp TableTest.cpp Utf8CharTest.cpp
  ../testMain.cpp)
target_link_libraries(${TARGET} PRIVATE ${LIB_PREFIX}kana gtest)
