#! /usr/bin/env bash
#
# This test is only invoked if the CMAKE flag US_USE_DETERMINISTIC_BUNDLE_BUILDS is set.
# Test to check that 2 different zip files created with exactly the same content result in identical
# zip files such that their sha 512 checksums are the same. This will fail if the resource compiler
# embeds anything like date or time stamps.
#
# this is *nix only. For windows, see basic_test.bat

RC_EXE=$1
shift

rm -f test_file.txt test1.zip test2.zip

touch test_file.txt
$RC_EXE --out-file test1.zip --res-add test_file.txt --bundle-name test_bundle
sleep 2
$RC_EXE --out-file test2.zip --res-add test_file.txt --bundle-name test_bundle

test1_shasum=`shasum -b -a 512 test1.zip | awk '{print $1}'`
test2_shasum=`shasum -b -a 512 test2.zip | awk '{print $1}'`

([ -f test1.zip ] && [ -f test2.zip ] && [ $test1_shasum == $test2_shasum ] && exit 0) \
    || (echo "sha 512 chechsums do not match eventhough deterministic bundle builds are enabled" && exit 1)
