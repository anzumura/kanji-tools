add_executable(${TARGET} ConverterTest.cpp KanaTest.cpp MBCharTest.cpp
  TableTest.cpp ../testMain.cpp)
target_link_libraries(${TARGET} PRIVATE ${LIB_PREFIX}kana gtest)
