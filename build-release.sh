#! /bin/bash
mkdir build-release
cd build-release
qmake ../ffs2play.pro CONFIG+=release
make clean
make -j3
./ffs2play
