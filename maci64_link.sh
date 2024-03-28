#! /usr/bin/env bash
#
# According to: https://blog.llvm.org/2019/11/deterministic-builds-with-clang-and-lld.html
# 
# "macOS’s libtool and ld64 also insist on writing timestamps into their outputs. You can set the
# environment variable ZERO_AR_DATE to 1 in a wrapper to make their output deterministic"
#
# This is a small wrapper script invoked by the build system on macs only that sets the environnment
# variable as described above and then invokes the linker.
#
# $@ holds the linker command an all its arguments.
export ZERO_AR_DATE=1
$@
