#/bin/bash

for i in 2 4 6 8 10
do
    # run 4 times
    for j in 1 2 3 4
    do
        yhbatch -p thcp1 -n 1 ./run.sh $i
    done
    # sleep i*5 seconds
    sleep $i*5
done