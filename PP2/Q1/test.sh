#!/bin/bash
gcc matrix_print.c -o ./matrix_print.exe -fopenmp -w
for j in 1 2 4 8
do
	for((i=0; i< 10; i+=1))  
	do
		./matrix_print.exe $i $j
    done
	for((i=10; i< 100; i+=10))  
	do
		./matrix_print.exe $i $j
    done
	for((i=100; i< 2000; i+=100))  
	do
		./matrix_print.exe -N $i -T $j
    done
done
make clean