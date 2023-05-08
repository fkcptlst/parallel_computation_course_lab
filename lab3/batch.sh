#!/usr/bin/bash

N_steps=$1
num_node=$2
mat_dim=$3

for n in 2 3 5 7 9 11
do
     # wait if queue too long
     while [ $(squeue -h | wc -l) -ge 1 ]
     do
         sleep 1
     done
     echo "num_proc=$n"
     ./minibatch.sh "$N_steps" "$num_node" "$n" "$mat_dim"
done

echo "done."
