#! /usr/bin/env bash
#

rm -f test_file.txt test1.zip test2.zip

touch test_file.txt
$RC_EXE --out-file test1.zip --res-add test_file.txt --bundle-name test_bundle
sleep 2
$RC_EXE --out-file test2.zip --res-add test_file.txt --bundle-name test_bundle

test1_shasum=`$SHA_SUM_CMD test1.zip | awk '{print $1}'`
test2_shasum=`$SHA_SUM_CMD test2.zip | awk '{print $1}'`

([ -f test1.zip ] && [ -f test2.zip ] && [ $test1_shasum == $test2_shasum ] && exit 0) \
    || (echo "sha 512 chechsums do not match eventhough deterministic bundle builds are enabled" && exit 1)
