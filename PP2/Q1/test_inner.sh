#!/bin/bash
NAME=matrix_inner_print
C="$NAME.c"
EXE="$EXE.c"
gcc "$C" -o "$NAME" -fopenmp -w
for j in 1 2 4 8
do
	for((i=0; i< 10; i+=1))  
	do
		./"$NAME" -N $i -T $j
    done
	for((i=10; i< 100; i+=10))  
	do
		./"$NAME" -N $i -T $j
    done
	for((i=100; i< 2000; i+=100))  
	do
		./"$NAME" -N $i -T $j
    done
done
make clean