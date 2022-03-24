#!/usr/bin/env bash

# add this script as an 'Execute shell' build step after the actual build to
# generate reports for Jenkins post build actions (JUnit and Cobertura)

# Example 'Execute shell' build commands for a 'Clang' 'Debug' build:
#
# cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -Bbuild
# cmake --build build -j 10

# Some global Jenkins env vars that can help the build are:
#   PATH=/opt/homebrew/bin:$PATH
#   CMAKE_GENERATOR="Unix Makefiles"
#   CMAKE_EXPORT_COMPILE_COMMANDS=TRUE

# remove coverage results from previous runs
find build/src -name '*.gcda' -exec rm {} \;

declare -r r=../../..
cd build/tests
for i in *; do
  cd $i
  ./${i}Test --gtest_output=xml
  gcovr -x -r$r -f$r/src -f$r/include > coverage.xml
  cd ..
done

# set the following values for the actions:
# - Publish JUnit test result:
#   set 'Test report XMLs' to: 'build/tests/*/test*.xml'
# - Publish Cobertura Coverage:
#   set 'Cobertura xml report pattern' to: 'build/tests/*/coverage.xml'

# googletest docs: https://google.github.io/googletest/
# gcovr docs: https://gcovr.com/en/stable/
# Cobertura plugin docs: https://plugins.jenkins.io/cobertura/