#! /usr/bin/env bash
#
wget https://sourceware.org/pub/valgrind/valgrind-3.16.1.tar.bz2
tar -xjf valgrind-3.16.1.tar.bz2
pushd valgrind-3.16.1
./configure --prefix=/usr/local
make
sudo make install
ccache --clear
popd
valgrind --version

