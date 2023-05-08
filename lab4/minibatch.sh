#!/usr/bin/bash

n=$1
# run 4 times
for j in 1 2 3 4
do
    yhbatch -p thcp1 -N "$n" -n "$n" ./run.sh "$n"
done
