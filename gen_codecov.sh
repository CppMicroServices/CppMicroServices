#!/bin/bash
GCOV_EXE=${MY_COVERAGE} # gcov and g++ versions must match

for filename in `find . -name "*.cpp"`;
do
  $GCOV_EXE -o . $filename;
done
