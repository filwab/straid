#!/bin/bash

# 定义全盘顺序写入的数据量，这里是整个盘。
size_bytes=$((16*1024*1024*1024))  # 16GB转换为字节

block_size=$((4*1024*1024))

# 定义fio进行随机4k写入的数据量
fio_size="8G"
jobs_num="2"

size="10G"

# 关闭延迟
./femu-config.sh 4

# 遍历所有nvme*n1设备
for device in /dev/nvme*n1; do
    echo "开始对 $device 设备进行全盘顺序写入..."
    # dd if=/dev/urandom of="$device" bs=$block_size count=$((size_bytes/block_size))  oflag=direct status=progress
    fio --name=seq_write_test --ioengine=libaio --numjobs=$jobs_num  --rw=write --bs=1M --direct=1  --size=$fio_size --group_reporting  --filename="$device"

    echo "$device 设备写入完成!"
done

#开启延迟
./femu-config.sh 3

echo "所有NVMe盘的顺序写入过程完成。"
