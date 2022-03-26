#!/usr/bin/env bash

# add this script as an 'Execute shell' build step after the actual build to
# generate reports for Jenkins post build actions (JUnit and Cobertura)

# Example 'Execute shell' build commands for a 'Clang' 'Release' build:
#
#   cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ -Bbuild
#   cmake --build build -j 10
#
# For 'Debug' build types, add '-DCODE_COVERAGE' to include code coverage flags

# Some global Jenkins env vars that can help the build are:
#   PATH=/opt/homebrew/bin:$PATH
#   CMAKE_GENERATOR="Unix Makefiles"
#   CMAKE_EXPORT_COMPILE_COMMANDS=TRUE

for i in src tests; do
  i=build/$i
  if [[ ! -d $i ]]; then
    echo "'$i' directory not found"
    exit 1
  fi
done

cd build
# only make coverage reports if there are .gcno files (so with -DCODE_COVERAGE)
if [[ -n $(find src -name *.gcno | head -1) ]]; then
  # remove coverage results from previous runs
  find . -name *.gcda -exec rm {} \;
  declare -r r=../../..
fi

cd tests
for i in *; do
  ./$i/${i}Test --gtest_output=xml
done
if [[ -n $r ]]; then
  for i in *; do
    # 'gcovr' needs to run in the same directory as the executable
    cd $i && gcovr -x -r$r -f$r/src -f$r/include > coverage.xml && cd ..
  done
fi

# set the following values for the actions:
# - Publish JUnit test result:
#   set 'Test report XMLs' to: 'build/tests/*.xml'
# - Publish Cobertura Coverage:
#   set 'Cobertura xml report pattern' to: 'build/tests/*/*.xml'
#   set 'Source Encoding' (in Advanced options) to: 'UTF-8'

# googletest docs: https://google.github.io/googletest/
# gcovr docs: https://gcovr.com/en/stable/
# Cobertura plugin docs: https://plugins.jenkins.io/cobertura/
