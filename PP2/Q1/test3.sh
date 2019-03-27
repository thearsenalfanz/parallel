#!/bin/bash
NAME=matrix_all_test
C="$NAME.c"
EXE="$NAME.exe"
gcc "$C" -o "$EXE" -fopenmp -w
for j in 1 2 4 8
do
	for((i=10; i< 100; i+=10))  
	do
		./"$EXE" -N $i -T $j
    done
	for((i=100; i< 1500; i+=100))  
	do
		./"$EXE" -N $i -T $j
    done
done
make clean