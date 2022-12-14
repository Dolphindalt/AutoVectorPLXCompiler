#!/bin/bash
input=$1
if [ ! $1 ]
then
    echo "Please provide a valid input file"
else
    cd build
    ./Compiler.out ${input} -v
    gcc -c ../std/stdio.s output.s && ld stdio.o output.o
fi