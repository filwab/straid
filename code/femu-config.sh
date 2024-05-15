#!/bin/bash
#-------------------------------------------------------------------------------
# Script to dynamically config (enable/disable) FEMU features
#-------------------------------------------------------------------------------

FEMU_FLIP_CMD_OPCODE="0xef"

cmd=$1

SSD_NUM=`ls -l /dev/nvme*n1 | wc -l`
SSD_MAX=`expr ${SSD_NUM} - 1`

if [[ $# != 1 ]]; then
    echo ""
    echo "Usage: $(basename $0) [1-4,8-9,13,19-20]"
    echo "Latency:"
    echo "  1 : enable_gc_delay,      2 : disable_gc_delay"
    echo "  3 : enable_delay_emu,     4 : disable_delay_emu"
    echo "IODA:"
    echo "  8 : enable_gc_sync,       9 : disable_gc_sync"
    echo "IODA TW:"
    echo "  13: tw=100ms"
    echo "READ IO CONTROL:"
    echo "  19: fast_fail_switch"
    echo "LOG:"
    echo "  20: Straid log switch"
    echo "control:"
    echo "  21 : Base mode,      22 : IODA mode"
    echo "  23 : Print courr gcs,     24 : Utiliz Inter ssd     25 : Show all  values"
    echo "  26 : buffer_mode(+group_gc),     27 :group_gc_switch     28 : Group  info"
    echo "  29 : gc-steering-mod"
    exit
fi

for i in $(seq 0 ${SSD_MAX}); do
    sudo nvme admin-passthru --opcode=${FEMU_FLIP_CMD_OPCODE} --cdw10=${cmd} /dev/nvme${i}n1  >/dev/null 2>&1
done
