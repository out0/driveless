#! /bin/sh
rm -rf build
mkdir  -p build/net
cd build && cmake ..
make
mv planner ..
