#!/bin/bash
mkdir build
cd build
cmake ..
make
gcc ..std/stdio.s -shared -o pl0_stdlib.so
cp pl0_stdlib.so /usr/local/lib
cp Compiler.out /usr/local/bin
