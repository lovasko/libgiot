#!/bin/sh

# pop_cnt
clang -o pop_cnt.so -shared pop_cnt.c
giotgen -i 1 \
        -o 1 \
        -f pop_cnt \
        -d "Population count" > pop_cnt.giot
giotinfo pop_cnt.giot
giotls pop_cnt.giot -l 10 -f bin 

# max3
clang -o max3.so -shared max3.c 
giotgen -i 3 \
        -o 1 \
        -f max3 \
        -d "Maximum of 3 bytes" > max3.giot 
giotinfo max3.giot 
giotls max3.giot -l 10 -f hex


