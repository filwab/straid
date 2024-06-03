import matplotlib.pyplot as plt
import numpy as np
import re
from collections import Counter

input_path = '../Traces/8ssd_trace/10m_range/'
output_path = '../Traces/off_dis/'

#统计负载偏移变化分布
def plot_offset_distribution(lfile):
    offsets = []
    
    # 读取日志文件并提取偏移地址
    log_file_path = input_path + lfile

    with open(log_file_path, 'r') as file:
        for line in file:
            parts = line.strip().split()
            if len(parts) != 3:
                continue  # 忽略格式不正确的行
            # if parts[0] == 'R':
            #     continue
            offset = int(parts[1])
            offsets.append(offset)
    
    # 将偏移地址转换为MB单位
    offsets_in_mb = [offset / (1024 * 1024) for offset in offsets]
    
    # 绘图
    plt.figure(figsize=(10, 6))
    plt.plot(offsets_in_mb, marker='o',markersize = 1)
    # plt.scatter(range(len(offsets_in_mb)), offsets_in_mb, alpha=0.5, s=0.4)
    plt.title(lfile[:-4]+"_OffsetDistribution")
    plt.xlabel("Request Number")
    plt.ylabel("Offset (MB)")
    plt.grid(True)
    plt.savefig(output_path + lfile[:-4]+'_off_dis.png')


def static_logfile(lfile):
    # 初始化计数器
    read_count = 0
    write_count = 0
    total_read_io = 0
    total_write_io = 0

    log_file_path = input_path + lfile
    with open(log_file_path, 'r') as logfile:
        for line in logfile:
            parts = re.split(r'\s+', line.strip())
            if len(parts) != 3:
                continue
            
            io_type = parts[0]
            io_size = int(parts[2])
            
            if io_type == 'R':
                read_count += 1
                total_read_io += io_size
            elif io_type == 'W':
                write_count += 1
                total_write_io += io_size

    # 计算总的IOsize，以GB为单位
    total_read_io_gb = total_read_io / (1024 * 1024 * 1024)
    total_write_io_gb = total_write_io / (1024 * 1024 * 1024)

    # 输出统计结果t
    print("\n\n result of "+lfile+" : ")
    print(f'读请求数量: {read_count}')
    print(f'写请求数量: {write_count}')
    print(f'读请求总的IOsize (GB): {total_read_io_gb:.2f}')
    print(f'写请求总的IOsize (GB): {total_write_io_gb:.2f}')

    #close file
    logfile.close()

    # 进阶统计读写，io值种类，并将值除以1024以KB显示
    print('进阶统计 (IOsize以KB为单位):')
    io_sizes_kb = Counter()

    with open(log_file_path, 'r') as logfile:
        for line in logfile:
            parts = re.split(r'\s+', line.strip())
            if len(parts) != 3:
                continue
            
            io_size_kb = int(parts[2]) / 1024
            io_size_kb = round(io_size_kb, 2)
        
            io_sizes_kb[io_size_kb] += 1

    # for size, count in sorted(io_sizes_kb.items()):
    #     print(f'{count} 次: {size} KB')
    # #只打印io_sizes_kb前10个值
    # print("io_sizes_kb前10个值:")
    # i = 0
    # for size, count in sorted(io_sizes_kb.items()):
    #     print(f'{count} 次: {size} KB')
    #     i += 1
    #     if i == 10:
    #         break
    # 打印出现次数最多的十种值
    print('出现次数最多的十种IOsize (以KB为单位):')
    for size, count in io_sizes_kb.most_common(10):
        print(f'{count} 次: {size} KB')
    
    
    #close file
    logfile.close()


files = ["hm0.log","mds1.log","rsrch0.log","wdev0.log"]

for file in files:
    static_logfile(file)