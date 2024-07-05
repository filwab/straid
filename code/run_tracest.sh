#!/bin/bash

thread=32  # set the number of running threads
chunksize=64 # set the chunk size of straid to (chunksize * KB)

trace_path8="./Traces/8ssd_trace/aliTrace/"
trace_path7="./Traces/7ssd/"
# wamr
# trace_file="./Traces/8ssd_trace/8_warm_90g.log"
# trace_file="./Traces/8ssd_trace/8_warm_20g.log"
# trace_file="./Traces/read_80g.log"

# trace_file=" hm0.log   mds1.log   rsrch0.log  wdev0.log"
# trace_file="./Traces/8ssd_trace/10m_range/wedevs0.log"
# trace_file="./Traces/8ssd_trace/aliTrace 112.log    131.log  188.log  251.log  295.log    64.log
# trace_file="./Traces/8ssd_trace/aliTrace/504.log"

trace_group=("64.log" "112.log" "131.log" "188.log" "251.log" "295.log")
trace_groupio=("fileserver_1.log" "nload_160g_r.log" "nload_160g.log" "7_ycsb02.log")
# trace_group=("295.log")

for file in "${trace_groupio[@]}"
do
    trace_file=${trace_path7}${file}
    ./femu-config.sh 23
    ./femu-config.sh 25
    echo "Running trace file: ${trace_file}"
    sed -in-place -e '/^#define NUM_THREADS/ c \#define NUM_THREADS ('${thread}")" ./include/define.h
    sed -in-place -e '/^#define SCHUNK_SIZE/ c \#define SCHUNK_SIZE ('${chunksize}" * KB)" ./include/define.h
    bear make -j8 > /dev/null 2>&1

    echo "" > ./results/ST_trace_results.txt

    ./bin/trace_st "$trace_file"

    echo ""
    echo "========================================================="
    echo ""

done

# sed -in-place -e '/^#define NUM_THREADS/ c \#define NUM_THREADS ('${thread}")" ./include/define.h
# sed -in-place -e '/^#define SCHUNK_SIZE/ c \#define SCHUNK_SIZE ('${chunksize}" * KB)" ./include/define.h
# bear make -j8

# echo "" > ./results/ST_trace_results.txt

# ./bin/trace_st "$trace_file"
