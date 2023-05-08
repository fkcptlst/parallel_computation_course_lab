#!/bin/bash

# params
N=100
num_thread=$1

# run
echo "==========params=========="
echo "N=$N"
echo "num_thread=$num_thread"

echo "==========results_serial=========="
time yhrun -p thcp1 -n 1 ./serial.o $N 960 3 &>> results.log

echo "==========results_parallel=========="
time yhrun -p thcp1 -n 1 -c 8 ./parallel.o $N 960 3 $num_thread &>> results.log