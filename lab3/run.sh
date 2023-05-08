#!/bin/bash

module load GCC/9.3.0 mpich/mpi-x-gcc9.3.0

# params
N=$1
num_node=$2
num_proc=$3
mat_dim=$4

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
time yhrun -p thcp1 -N "$num_node" -n "$num_proc" ./parallel.o "$N" "$mat_dim" 3 &>> results.log