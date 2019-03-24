#!/bin/bash
  for((i=100; i< 2000; i+=100))
  do
    for j in 1 2 4 8
    do
      ./gauss_test.exe $i $j
    done
  done