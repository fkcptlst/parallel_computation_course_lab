#!/bin/bash

# params
N=10000000
num_thread=$1

# run
echo "==========params=========="
echo "N=$N" 
echo "num_thread=$num_thread"

echo "==========lab1_integral_serial==========" 
time yhrun -p thcp1 -n 1 ./lab1_integral_serial.o $N &>> results.log

echo "==========lab1_integral_parallel==========" 
time yhrun -p thcp1 -n 1 -c 8 ./lab1_integral_parallel.o $N $num_thread &>> results.log