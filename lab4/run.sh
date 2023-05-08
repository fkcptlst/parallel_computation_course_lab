#!/bin/bash

module load GCC/9.3.0 mpich/mpi-x-gcc9.3.0

# params
N=100
num_proc=$1

# run
echo "==========params=========="
echo "N=$N"
echo "num_proc=$num_proc"

echo "==========results_serial=========="
echo ""
echo "real	0m0.000s"
echo "user	0m0.000s"
echo "sys	0m0.000s"

echo "==========results_parallel=========="
time yhrun -p thcp1 -N $num_proc -n $num_proc ./parallel.o $N 960 3 &>> results.log