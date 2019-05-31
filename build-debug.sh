#! /bin/bash
mkdir build-debug
cd build-debug
qmake ../ffs2play.pro CONFIG+=debug
make clean
make -j3
./ffs2play
