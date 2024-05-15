#!/bin/bash

thread=32  # set the number of running threads
chunksize=64 # set the chunk size of straid to (chunksize * KB)

trace_path5="./Traces/5ssd/"
trace_path7="./Traces/7ssd/"
trace_path9="./Traces/9ssd/"

trace_group5=("fileserver_1.log" "nload_80g.log" "nload_80g_r.log" "ycsb02.log")
trace_group7=("fileserver_1.log" "nload_160g.log" "nload_160g_r.log" "7_ycsb02.log")
trace_group9=("fileserver_1.log" "nload_320g.log" "nload_320g_r.log" "9_ycsb02.log")

trace_group=("9_ycsb02.log")

for file in "${trace_group[@]}"
do 
    trace_file=${trace_path9}${file}
    ./femu-config.sh 23
    echo "Running trace file: ${trace_file8}"
    sed -in-place -e '/^#define NUM_THREADS/ c \#define NUM_THREADS ('${thread}")" ./include/define.h
    sed -in-place -e '/^#define SCHUNK_SIZE/ c \#define SCHUNK_SIZE ('${chunksize}" * KB)" ./include/define.h
    bear make -j8

    echo "" > ./results/ST_trace_results.txt

    ./bin/trace_st "$trace_file"

    mv ./results/ST_trace_results.txt ./results/ST_trace_results_ioda_${file}.txt

    echo "Finished trace file: ${trace_file}" >> ./results/statistics.txt
    ./femu-config.sh 23   

done

# sed -in-place -e '/^#define NUM_THREADS/ c \#define NUM_THREADS ('${thread}")" ./include/define.h
# sed -in-place -e '/^#define SCHUNK_SIZE/ c \#define SCHUNK_SIZE ('${chunksize}" * KB)" ./include/define.h
# bear make -j8

# echo "" > ./results/ST_trace_results.txt

# ./bin/trace_st "$trace_file8"