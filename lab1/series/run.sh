#!/bin/bash

# params
N=10000000000
num_thread=10

# run
echo "==========params==========" > lab1_series.log
echo "N=$N" >> lab1_series.log
echo "num_thread=$num_thread" >> lab1_series.log

echo "==========lab1_series_serial==========" >> lab1_series.log
time yhrun -p thcp1 -n 1 ./lab1_series_serial.o $N &>> lab1_series.log

echo "==========lab1_series_parallel==========" >> lab1_series.log
time yhrun -p thcp1 -n 1 -c $num_thread ./lab1_series_parallel.o $N $num_thread &>> lab1_series.log