#!/bin/sh
set -e
if [ ! -d "$HOME/cache" ]; then
  mkdir $HOME/cache;
fi

# check to see if CMake is cached
if [ ! -f "$HOME/cache/bin/ctest" ]; then

  wget --no-check-certificate https://cmake.org/files/v2.8/cmake-2.8.12.2.tar.gz -O /tmp/cmake.tar.gz;
  tar -xzvf /tmp/cmake.tar.gz -C /tmp;
  cd /tmp/cmake-2.8.12.2;
  ./configure --prefix=$HOME/cache;
  make -j;
  make install;
else
  echo "Using cached bin dir: $HOME/cache/bin";
  ls -la $HOME/cache/bin;
  echo "Using cached CMake installation:";
  cmake -version;
fi
