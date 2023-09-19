#! /bin/bash

# process="io_uring-sq"
# processid=$(ps -ef | grep ${process} | grep -v grep | awk '{print $2}')
# echo "process name is ${process}, tid is ${processid}"

# perf record -k -m io_uring -e branches -g graph --filter 'io_function'


FLAMEGRAPH_SCRIPTS="/home/femu/StRAID/straid/scripts/FlameGraph"


rm -rf perf.*
sudo perf record -e cycles  ./bin/trace_st
sleep 1
sudo perf script -i perf.data > perf.unfold
perl $FLAMEGRAPH_SCRIPTS/stackcollapse-perf.pl perf.unfold > perf.folded
perl $FLAMEGRAPH_SCRIPTS/flamegraph.pl perf.folded > perf-straid.svg
echo "flame svg file generated: perf-straid.svg"


killall  trace_st

sleep 2
