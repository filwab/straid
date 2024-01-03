#!/bin/bash

# 定义fio进行随机4k写入的数据量
fio_size="1G"

# 定义fio进行顺序写入的数据量
size="10G"

# 定义设备文件名
device="/dev/nvme0n1"

echo "开始对 ${device} 设备进行全盘顺序写入..." 
# dd if=/dev/urandom of="$device" bs=$block_size count=$((size_bytes/block_size))  oflag=direct status=progress
fio --name=seq_write_test --ioengine=libaio --iodepth=16 --rw=write --bs=1M --direct=1  --numjobs=8 --thread --group_reporting --filename="$device" 

echo "开始对${device} 设备进行随机4k写入..."
fio --name=random_write_test --ioengine=libaio --iodepth=4 --rw=randwrite --bs=4k --direct=1 --size=$fio_size --numjobs=4 --thread --group_reporting --filename="$device"  --randrepeat=1 

echo "${device} 设备写入完成!"