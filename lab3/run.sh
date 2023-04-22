#!/bin/bash

module load GCC/9.3.0 mpich/mpi-x-gcc9.3.0

time yhrun -p thcp1 -N 2 -n 8 ./test.o &> run.log
