add_executable(${TARGET} ChoiceTest.cpp ConverterTest.cpp DisplaySizeTest.cpp
  KanaConvertTest.cpp KanaTest.cpp MBCharTest.cpp TableTest.cpp ../testMain.cpp)
target_link_libraries(${TARGET} PRIVATE ${LIB_PREFIX}kana gtest)
