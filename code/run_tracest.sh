#!/bin/bash

thread=32  # set the number of running threads
chunksize=64 # set the chunk size of straid to (chunksize * KB)

sed -in-place -e '/^#define NUM_THREADS/ c \#define NUM_THREADS ('${thread}")" ./include/define.h
sed -in-place -e '/^#define SCHUNK_SIZE/ c \#define SCHUNK_SIZE ('${chunksize}" * KB)" ./include/define.h
bear make -j8

echo "" > ./results/ST_trace_results.txt

./bin/trace_st
