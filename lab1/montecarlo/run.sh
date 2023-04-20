#!/bin/bash

# params
N=10000000000
num_thread=10

# run
echo "==========params==========" > lab1_montecarlo.log
echo "N=$N" >> lab1_montecarlo.log
echo "num_thread=$num_thread" >> lab1_montecarlo.log

echo "==========lab1_montecarlo_serial==========" >> lab1_montecarlo.log
time yhrun -p thcp1 -n 1 ./lab1_montecarlo_serial.o $N &>> lab1_montecarlo.log

echo "==========lab1_montecarlo_parallel==========" >> lab1_montecarlo.log
time yhrun -p thcp1 -n 1 -c $num_thread ./lab1_montecarlo_parallel.o $N $num_thread &>> lab1_montecarlo.log