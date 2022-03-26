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
  # remove coverage results from previous runs before running tests
  find . -name *.gcda -exec rm {} \;
  if otool -L $(ls tests/*/*Test | head -1) | grep -q /lib/gcc/; then
    cov=gcov-11
  else
    cov=gcov
  fi
fi

cd tests
for i in *; do
  cd $i
  ./${i}Test --gtest_output=xml
  cd ..
done
if [[ -n $cov ]]; then
  cd ..
  gcovr --gcov-executable=$cov -x -r.. -f../src -f../include > coverage.xml
fi

# set the following values for the actions:
# - Publish JUnit test result:
#   set 'Test report XMLs' to: 'build/*/test*.xml'
# - Publish Cobertura Coverage:
#   set 'Cobertura xml report pattern' to: 'build/coverage.xml'
#   set 'Source Encoding' (in Advanced options) to: 'UTF-8'

# googletest docs: https://google.github.io/googletest/
# gcovr docs: https://gcovr.com/en/stable/
# Cobertura plugin docs: https://plugins.jenkins.io/cobertura/
