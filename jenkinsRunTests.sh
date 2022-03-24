#!/usr/bin/env bash

# add this script as an 'Execute shell' build step after the actual build

# Example 'Execute shell' build commands for a 'Clang' 'Debug' build:
#
# mkdir -p build
# /opt/homebrew/bin/cmake -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE \
#   -DCMAKE_BUILD_TYPE:STRING=Debug \
#   -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/clang++ \
#   -S"$WORKSPACE" -B"$WORKSPACE/build" -G "Unix Makefiles"
# /opt/homebrew/bin/cmake --build build -j 10

# remove line-count results from previous test coverage runs
find build/src -name '*.gcda' -exec rm {} \;

cd build/tests
declare -r r=../../..
for i in *; do
  cd $i

  # generate report for 'Publish JUnit test result' post build action
  ./${i}Test --gtest_output="xml:results.xml"
  # set 'Test report XMLs' to: build/tests/*/results.xml

  # generate report for 'Publish Cobertura Coverage' post build action
  /opt/homebrew/bin/gcovr -x -r $r -f $r/src -f $r/include > coverage.xml
  # set 'Cobertura xml report pattern' to: build/tests/*/coverage.xml

  cd ..
done
