#!/bin/sh
set -e
if [ ! -d "$HOME/cache" ]; then
  mkdir $HOME/cache;
fi

# check to see if CMake is cached
if [ ! -f "$HOME/cache/bin/ctest" ]; then

  wget --no-check-certificate https://cmake.org/files/v2.8/cmake-2.8.12.2-Linux-i386.tar.gz -O /tmp/cmake.tar.gz
  tar -xzvf /tmp/cmake.tar.gz -C /tmp
  mv /tmp/cmake-2.8.12.2-Linux-i386/* $HOME/cache/
else
  echo 'Using cached CMake installation.';
fi
