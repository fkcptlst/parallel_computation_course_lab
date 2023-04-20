#/bin/bash
###
 # @Date: 2023-04-05 10:53:32
 # @LastEditors: Lcf
 # @LastEditTime: 2023-04-05 10:58:49
 # @FilePath: \labs\lab2\batch.sh
 # @Description: default
### 

for i in 2 4 6 8 10
do
    # run 4 times
    for j in 1 2 3 4
    do
        yhbatch -p thcp1 -n 1 ./run.sh $i
        # wait if yhq | wc -l >= 4
        while [ $(yhq | wc -l) -ge 4 ]
        do
            sleep 1
        done
    done
done