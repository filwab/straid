#!/bin/bash

thread=32  # set the number of running threads
chunksize=64 # set the chunk size of straid to (chunksize * KB)

# trace_file="./Traces/9_warm.log hm0.log   mds1.log   rsrch0.log  wdev0.log"
trace_file="./Traces/8ssd_trace/10m_range/wdev0.log"

sed -in-place -e '/^#define NUM_THREADS/ c \#define NUM_THREADS ('${thread}")" ./include/define.h
sed -in-place -e '/^#define SCHUNK_SIZE/ c \#define SCHUNK_SIZE ('${chunksize}" * KB)" ./include/define.h
bear make -j8

echo "" > ./results/ST_trace_results.txt

./bin/trace_st "$trace_file"
