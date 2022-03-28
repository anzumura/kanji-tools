#!/usr/bin/env bash

declare -r program="jenkinsRunTests.sh"

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

function log() {
  echo "[$program] $1"
}

function changeDir() {
  if [[ ! -d $1 ]]; then
    log "directory not found: '$PWD/$1' - exiting"
    exit 1
  fi
  cd $1
  log "current directory: $PWD"
}

log "PATH=$PATH"

changeDir build/tests
declare -r testTop=$PWD

# only make coverage reports if there are .gcno files (so with -DCODE_COVERAGE)
if [[ -n $(find . -name *.gcno | head -1) ]]; then
  # remove coverage results from previous runs before running tests
  log "detected a coverage build"
  log "- removing *.gcda files"
  find .. -name *.gcda -exec rm {} \;
  if otool -L $(ls ./*/*Test | head -1) | grep -q /lib/gcc/; then
    cov=gcov-11
  else
    cov=gcov
  fi
  log "- will use gcov executable: $(which $cov)"
fi

for i in *; do
  changeDir $testTop/$i
  ./${i}Test --gtest_output=xml
done

if [[ -n $cov ]]; then
  changeDir $testTop/..
  log "running: $(which gcovr)"
  # 'gcovr' 5.0 worked fine for both Clang and GCC, but version 5.1 gets a few
  # parse errors for GCC which don't seem to affect the overall coverage so for
  # now use '--gcov-ignore-parse-errors' to allow the report to get generated
  gcovr --gcov-executable=$cov -x -r.. -f../libs --gcov-ignore-parse-errors \
    --exclude-unreachable-branches --exclude-throw-branches >coverage.xml
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
