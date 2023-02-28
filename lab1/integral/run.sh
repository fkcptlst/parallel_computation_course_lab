#!/bin/bash

# params
N=10000000000
num_thread=10

# run
echo "==========params==========" > lab1_integral.log
echo "N=$N" >> lab1_integral.log
echo "num_thread=$num_thread" >> lab1_integral.log

echo "==========lab1_integral_serial==========" >> lab1_integral.log
time yhrun -p thcp1 -n 1 ./lab1_integral_serial.o $N &>> lab1_integral.log

echo "==========lab1_integral_parallel==========" >> lab1_integral.log
time yhrun -p thcp1 -n 1 -c $num_thread ./lab1_integral_parallel.o $N $num_thread &>> lab1_integral.log