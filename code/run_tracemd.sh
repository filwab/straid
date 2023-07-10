#!/bin/bash

thread=32   # set the number of I/O-issuing threads
mdtype="5"
mdpath="/dev/md5"

sed -in-place -e '/^#define NUM_THREADS/ c \#define NUM_THREADS ('${thread}")" ./include/define.h
make -j4

echo "" > ./results/MD_trace_results.txt

echo 64 > /sys/block/md5/md/group_thread_cnt
echo 32768 > /sys/block/md5/md/stripe_cache_size
cat /sys/block/md5/md/group_thread_cnt
cat /sys/block/md5/md/stripe_cache_size

./bin/trace_md ${mdtype} ${mdpath}
