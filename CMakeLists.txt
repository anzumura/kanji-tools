cmake_minimum_required(VERSION 3.14..3.22)
project(kanji_tools VERSION 1.0
                    DESCRIPTION "Tools for working with Japanesse Kana and Kanji"
                    LANGUAGES CXX)

### Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
  message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file).")
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_CXX_STANDARD 20)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # enable some warnings and turn them into errors (skip -Wdeprecated if using boost)
    set(WARN_FLAGS "-Werror -Wall -Wextra -Wunused -Wdeprecated -Wunreachable-code-aggressive")
    # disable a warning (add -Wno-unused-variable if using boost)
    set(WARN_FLAGS "${WARN_FLAGS} -Wno-logical-op-parentheses")
    set(SANITIZER_FLAGS "-fsanitize=address -fno-omit-frame-pointer -O1 -fno-optimize-sibling-calls")
    # need to set env for after stack reporting: ASAN_OPTIONS=detect_stack_use_after_return=1
    # set(SANITIZER_STACK_FLAGS "-fsanitize-address-use-after-scope")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SANITIZER_FLAGS} ${SANITIZER_STACK_FLAGS} ${WARN_FLAGS}")
  endif()
  # Use the following to override the linker
  # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold")
  include(CTest)
endif()

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.11.0
)
FetchContent_MakeAvailable(googletest)

### ICU
# icu build guide: https://unicode-org.github.io/icu/userguide/icu4c/build.html
# icu download: https://github.com/unicode-org/icu/releases - get 'Source code (tar.gz)'
#   tar -xzf Downloads/icu-release-70-1.tar
#   cd icu-release-70-1/icu4c/source
#   ./runConfigureICU MacOSX --prefix=/opt/icu
#   sudo make install
#
# uncomment the following lines to add ICU
# link_directories(BEFORE /opt/icu/lib)
# include_directories(BEFORE /opt/icu/include)

### Boost
# boost guide: https://www.boost.org/doc/libs/1_78_0/more/getting_started
# boost download: https://www.boost.org/users/download - get 'tar.gz'
#   tar -xzf Downloads/boost_1_78_0.tar 
#   cd boost_1_78_0
#   ./bootstrap.sh --with-icu=/opt/icu --prefix=/opt/boost
#   sudo ./b2 install
#
# uncomment the following lines to add Boost
# find_package(Boost 1.78.0 REQUIRED regex locale)
# if(Boost_FOUND)
#   link_directories(${Boost_LINK_DIR})
#   include_directories(${Boost_INCLUDE_DIR})
#   message("-- Boost found: ${Boost_VERSION_STRING}")
# else()
#   message(FATAL_ERROR "Boost not found!")
# endif()

add_subdirectory(src/kana)  # source files for 'kana' lib (depends on 'utils')
add_subdirectory(src/kanji) # source files for 'kanji' lib (depends on 'kana')
add_subdirectory(src/quiz)  # source files for 'quiz' lib (depends on 'kanji')
add_subdirectory(src/stats) # source files for 'stats' lib (depends on 'kanji')
add_subdirectory(src/utils) # source files for 'utils' lib
add_subdirectory(apps)

if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING) AND BUILD_TESTING)
  add_subdirectory(tests/kana)
  add_subdirectory(tests/kanji)
  add_subdirectory(tests/quiz)
  add_subdirectory(tests/stats)
  add_subdirectory(tests/utils)
endif()
