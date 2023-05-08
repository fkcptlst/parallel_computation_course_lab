#!/usr/bin/bash

for n in 2 4 6 8 10
do
    # run 4 times
    for j in 1 2 3 4
    do
        yhbatch -p thcp1 -N $n -n $n ./run.sh $n
        # # wait if queue too long
        # while [ $(yhq -h | wc -l) -ge 4 ]
        # do
        #     sleep 1
        # done
    done
    sleep 120
done