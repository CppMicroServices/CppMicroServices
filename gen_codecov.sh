#!/bin/bash
GCOV_EXE=${MY_COVERAGE} # gcov and g++ versions must match

for filename in `find . | egrep '\.cpp'`;
do
  $GCOV_EXE -n -o . $filename > /dev/null;
done
