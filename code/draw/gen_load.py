import random

# 配置参数
KB = 1024 
MB = 1024*1024
GB = 1024*1024*1024

total_io_size = (30 * GB)  # 总 IO 大小，读写请求共享
offset_range = (0, 49 * GB)  # IO 请求的范围
system_capacity = (50 *  GB)  # 系统容量
write_ratio = 0.6  # 写操作比例

current_io_size = 0
written_data = []  # 记录已写入的数据区域

# 读写请求的iosize列表
rio_list = [4*KB ,128*KB, 64*KB]  # 读请求的iosize值列表
wio_list = [4*64*KB]  # 写请求的iosize值列表


# 创建并打开文件用于写入
with open("../Traces/load_1.log", "w") as file:
    
    while current_io_size < total_io_size:
        if random.random() < write_ratio or not written_data:
            # 执行写操作
            operation = 'W'
            iosize = random.choice(wio_list)
            offset = random.randint(offset_range[0], offset_range[1])
            written_data.append((offset, iosize))
            
        else:
            # 执行读操作
            operation = 'R'
            read_index = random.randint(0, len(written_data) - 1)
            offset, max_iosize = written_data[read_index]
            iosize = random.choice(rio_list)
  
        offset = (offset//iosize) * iosize
        current_io_size += iosize
        file.write(f"{operation}\t{offset}\t{iosize}\n")

        # 检查是否达到总 IO 大小目标
        if current_io_size >= total_io_size:
            break
        
# #  去掉读地址和写地址的前后关系--多线程下没办法顺序执行。        
# with open("../Traces/load_2.log", "w") as file:
    
#     while current_io_size < total_io_size:
#         if random.random() < write_ratio :
#             # 执行写操作
#             operation = 'W'
#             iosize = random.choice(wio_list)
#             offset = random.randint(offset_range[0], offset_range[1])
            
#         else:
#             # 执行读操作
#             operation = 'R'
#             offset = random.randint(offset_range[0], offset_range[1])
#             iosize = random.choice(rio_list)
  
#         offset = (offset//iosize) * iosize
#         current_io_size += iosize
#         file.write(f"{operation}\t{offset}\t{iosize}\n")

#         # 检查是否达到总 IO 大小目标
#         if current_io_size >= total_io_size:
#             break

