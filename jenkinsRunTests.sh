#!/usr/bin/env bash

declare -r program=jenkinsRunTests.sh topDir=$PWD testDir=$PWD/build/tests

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

# allow changing the report type via command line - here are some of the types:
# --txt: compact human-readable summaries
# --html: overview of all files
# --html-details: annotated source files
# --cobertura: (same as -x, the default for this script)
[[ $# -gt 0 ]] && report=$1 || report=-x

reportDir=build
reportFile=coverage.
# try to use an appropriate suffix for report file
case $report in
*html*)
  reportDir+=/html
  reportFile+=html
  ;;
*json* | --coveralls)
  reportDir+=/json
  reportFile+=json
  ;;
*csv*) reportFile+=csv ;;
*) reportFile+=xml ;;
esac

log "PATH=$PATH"

changeDir "$testDir"

# only make coverage reports if there are .gcno files (so with -DCODE_COVERAGE)
declare -r gcno='*.gcno'
if [[ -n $(find . -name $gcno | head -1) ]]; then
  # remove coverage results from previous runs before running tests - this is
  # faster than wiping the workspace or doing a full re-build, but it's fragile
  # if files are renamed, removed, etc.. (then a full re-build should be done)
  log "detected a coverage build"
  log "- remove .gcda and .gcov files"
  find .. \( -name '*.gcda' -o -name '*.gcov' \) -exec rm {} \;
  log "- remove .gcno files that don't have a corresponding .o file"
  find .. -name $gcno -exec bash -c '[[ -f ${1/.gcno/.o} ]] || rm "$1"' _ {} \;
  if otool -L $(ls ./*/*Test | head -1) | grep -q /lib/gcc/; then
    cov=gcov-11
  else
    cov=gcov
  fi
  log "- will use gcov executable: $(which $cov)"
fi

for i in *; do
  changeDir "$testDir/$i"
  ./${i}Test --gtest_output=xml
done

if [[ -n $cov ]]; then
  changeDir "$topDir"
  if [[ $reportDir != build ]]; then
    if [[ -d $reportDir ]]; then
      log "cleaning up files in: $reportDir"
      rm -rf $reportDir/*
    else
      mkdir $reportDir
    fi
  fi
  log "running: $(which gcovr)"
  # 'gcovr' 5.0 worked fine for both Clang and GCC, but version 5.1 gets a few
  # parse errors for GCC which don't seem to affect the overall coverage so for
  # now use '--gcov-ignore-parse-errors' to allow the report to get generated
  gcovr --gcov-executable=$cov $report $reportDir/$reportFile -d -flibs \
    --gcov-ignore-parse-errors --exclude-unreachable-branches \
    --exclude-throw-branches build/libs build/tests
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
