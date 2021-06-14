cmake_minimum_required(VERSION 3.14..3.20)
project(kanji VERSION 1.0
              DESCRIPTION "Kanji Lists"
              LANGUAGES CXX)

### Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
  message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file).")
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_EXTENSIONS OFF)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem /opt/homebrew/include")
  set(SANITIZER_FLAGS "-fsanitize=address -fno-omit-frame-pointer -O1 -fno-optimize-sibling-calls")
  # need to set env for after stack reporting: ASAN_OPTIONS=detect_stack_use_after_return=1
  # set(SANITIZER_STACK_FLAGS "-fsanitize-address-use-after-scope")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SANITIZER_FLAGS} ${SANITIZER_STACK_FLAGS}")
  include(CTest)
endif()

add_subdirectory(src)
