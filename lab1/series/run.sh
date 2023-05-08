#!/bin/bash

# params
N=10000000
num_thread=$1

# run
echo "==========params=========="
echo "N=$N"
echo "num_thread=$num_thread"

echo "==========lab1_series_serial=========="
time yhrun -p thcp1 -n 1 ./lab1_series_serial.o $N &>> lab1_series.log

echo "==========lab1_series_parallel=========="
time yhrun -p thcp1 -n 1 -c $num_thread ./lab1_series_parallel.o $N $num_thread &>> lab1_series.log