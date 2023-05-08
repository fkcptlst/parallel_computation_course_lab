#/usr/bin/bash

for i in 2 4 6 8 10
do
    # run 2 times
    for j in 1 2 3 4
    do
        yhbatch -p thcp1 -n 1 ./run.sh $i
         # wait if queue too long
         while [ $(squeue -h | wc -l) -ge 4 ]
         do
             sleep 1
         done
    done
done
