#!/bin/bash
name = matrix_inner_print
gcc $name.c -o ./$name.exe -fopenmp -w
for j in 1 2 4 8
do
	for((i=0; i< 10; i+=1))  
	do
		./$name.exe -N $i -T $j
    done
	for((i=10; i< 100; i+=10))  
	do
		./$name.exe -N $i -T $j
    done
	for((i=100; i< 2000; i+=100))  
	do
		./$name.exe -N $i -T $j
    done
done
make clean