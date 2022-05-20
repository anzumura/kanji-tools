cmake_minimum_required(VERSION 3.14..3.23)
project(kanji_tools VERSION 1.0
                    DESCRIPTION "Tools for working with Japanese Kana and Kanji"
                    LANGUAGES CXX)

### Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
  message(FATAL_ERROR "can't build in a source directory")
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_EXTENSIONS OFF) # use -std=c++xx instead of -std=g++xx
  include(CTest)
endif()

# enable warnings and turn them into errors (-W is the same as -Wextra, but it's
# deprecated for GCC so explicitly use -Wextra). -Wpedantic means 'Issue all the
# warnings demanded by strict ISO C++'. Links for more info:
# Clang: https://clang.llvm.org/docs/DiagnosticsReference.html
# GCC: https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
add_compile_options(-Wall -Wconversion -Werror -Wextra -Wextra-semi
  -Wignored-qualifiers -Wnonnull -Wold-style-cast -Wpedantic -Wsuggest-override)

# to turn off a warning for a section of code use the following ('GCC' works for
# both Clang and GCC):
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
# ... code with warning ...
#pragma GCC diagnostic pop

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # '-Wdeprecated' causes an error with GCC on line 918 in 'gtest-internal.h' so
  # add it for Clang here and add it for GCC after building googletest
  add_compile_options(-Wdeprecated -Wduplicate-enum -Wheader-hygiene
    -Wloop-analysis -Wshadow-all -Wsuggest-destructor-override
    -Wunreachable-code-aggressive)

  # turn off a warning
  add_compile_options(-Wno-logical-op-parentheses)

  set(SANITIZER_FLAGS "-fsanitize=address -fno-omit-frame-pointer \
-fno-optimize-sibling-calls")
  # need to set env for stack: ASAN_OPTIONS=detect_stack_use_after_return=1
  # string(APPEND SANITIZER_FLAGS "-fsanitize-address-use-after-scope")
  string(APPEND CMAKE_CXX_FLAGS_DEBUG " ${SANITIZER_FLAGS}")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  # Clang -Wall sets -Wmost which sets -Woverloaded-virtual, -Wunused, etc., and
  # -Wshadow-all sets -Wshadow (and more). GCC -Wall also sets things like
  # -Wunused (see links above), but the following don't seem to be included:
  add_compile_options(-Wnon-virtual-dtor -Woverloaded-virtual -Wshadow)

  # turn off a warning
  add_compile_options(-Wno-parentheses)
endif()
string(APPEND CMAKE_CXX_FLAGS_DEBUG " -O0")

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  # 1.11.0 was released on June 11, 2021 (or change to HEAD)
  GIT_TAG release-1.11.0
)
FetchContent_MakeAvailable(googletest)

### ICU
# icu build guide: https://unicode-org.github.io/icu/userguide/icu4c/build.html
# icu download: https://github.com/unicode-org/icu/releases - get tar.gz
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
#   include_directories(SYSTEM ${Boost_INCLUDE_DIR})
#   message("-- Boost found: ${Boost_VERSION_STRING}")
# else()
#   message(FATAL_ERROR "Boost not found!")
# endif()

# Need to add these after building googletest. 'switch-enum' happens even when a
# 'default' exists. '-Wswitch' (enabled by default) only warns about a missing
# enum value when there is no 'default'.
add_compile_options(-Wswitch-enum)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wcovered-switch-default)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  add_compile_options(-Wdeprecated -Wuseless-cast)
endif()

set(LIB_PREFIX kanjiTools_) # prefix for libs created by this project
set(LIBS kana kanji quiz stats utils) # list of libs created by this project

# add options for test coverage (only affects debug builds)
if(CODE_COVERAGE)
  string(APPEND CMAKE_CXX_FLAGS_DEBUG " --coverage -fno-elide-constructors \
-fno-inline")
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " -fprofile-abs-path")
  endif()
endif()

# add tests (there is one 'test' program per lib)
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
  foreach(LIB ${LIBS})
    set(TARGET ${LIB}Test)
    add_subdirectory(tests/${LIB})
    target_include_directories(${TARGET} PRIVATE tests/include)
    add_test(NAME ${LIB} COMMAND ${TARGET})
  endforeach()
endif()

# add libs
foreach(LIB ${LIBS})
  set(TARGET ${LIB_PREFIX}${LIB})
  add_subdirectory(libs/${LIB}/src)
  target_include_directories(${TARGET} PUBLIC libs/${LIB}/include)
endforeach()

# add this diagnostic flag here so that it only applies to main code (googletest
# uses '0' as a pointer in 'gtest.cc' line 2660 and operator<=> equality tests
# also produce this warning)
add_compile_options(-Wzero-as-null-pointer-constant)

# add 'main' programs
add_subdirectory(apps)

# check if Doxygen is installed
find_package(Doxygen)
if(DOXYGEN_FOUND)
  add_custom_target(doc_doxygen ALL
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/libs
    COMMAND ${DOXYGEN_EXECUTABLE} DoxyFile
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)
else(DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif(DOXYGEN_FOUND)
