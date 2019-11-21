#! /usr/bin/env bash
#
wget https://sourceware.org/pub/valgrind/valgrind-3.15.0.tar.bz2
tar -xjf valgrind-3.15.0.tar.bz2
pushd valgrind-3.15.0
./configure --prefix=/usr/local
make
sudo make install
ccache --clear
popd
valgrind --version

