#!/bin/bash
  for((i=100; i< 2000; i+=100))
  do
    for((j = 0; j < 4; j++))
    do
      ./gauss.exe $i $j
    done
  done