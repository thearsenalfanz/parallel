#!/bin/bash
gcc gauss_test.c -o ./gauss_test.exe -fopenmp -w
for j in 1 2 4 8
do
	for((i=100; i< 2000; i+=100))  
	do
		./gauss_test.exe $i $j
    done
done
make clean