#!/usr/bin/bash
N_steps=$1
num_node=$2
num_proc=$3
mat_dim=$4
# run 4 times
for j in 1 2 3 4
do
#    yhbatch -p thcp1 -N "$n" -n "$n" ./run.sh "$n"
    yhbatch -p thcp1 -N "$num_node" -n "$num_proc" ./run.sh "$N_steps" "$num_node" "$num_proc" "$mat_dim"
done
