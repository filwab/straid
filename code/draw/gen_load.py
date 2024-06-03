import random

# 配置参数
KB = 1024 
MB = 1024*1024
GB = 1024*1024*1024

total_io_size = (20 * GB)  # 总 IO 大小，读写请求共享
offset_range = (0, 85 * GB)  # IO 请求的范围
write_ratio = 1  # 写操作比例
chunk_size = (64 * KB)  # 系统块大小

current_io_size = 0
written_data = []  # 记录已写入的数据区域

# 读写请求的iosize列表
rio_list = [4*KB, 16*KB, 32*KB, 64*KB, 80*KB, 128*KB,]  # 读请求的iosize值列表
rio_weights = [35, 20, 5,  5, 10, 15] 
wio_list = [4*KB, 64*KB, 64*2*KB, 80*KB, 96*KB, 128*KB,256*KB,512*KB]  # 写请求的iosize值列表
wio_weights = [3, 1, 5, 1, 1, 1, 1, 1]

ran_wio = [4*KB, 16*KB, 64*KB, 80*KB, 96*KB]


# 创建并打开文件用于写入
with open("../Traces/8ssd_trace/8_warm_20g.log", "w") as file:
    
    while current_io_size < total_io_size:
        if random.random() < write_ratio or not written_data:
            # 执行写操作
            operation = 'W'
            # iosize = random.choices(wio_list, wio_weights)[0]
            offset = random.randint(offset_range[0], offset_range[1])
            offset = offset//(4*KB)*(4*KB)
            iosize = random.choices(ran_wio, [0.4, 0.2, 0.2, 0.1, 0.1])[0]
            # iosize = 1*MB
            # offset = random.randint(offset_range[0], offset_range[1])
            written_data.append((offset, iosize))
            
        else:
            # 执行读操作
            operation = 'R'
            read_index = random.randint(0, len(written_data) - 1)
            offset, max_iosize = written_data[read_index]
            iosize = random.choices(rio_list, rio_weights)[0]
  
        offset = (offset//chunk_size) * chunk_size
        current_io_size += iosize
        file.write(f"{operation}\t{offset}\t{iosize}\n")

        # 检查是否达到总 IO 大小目标
        if current_io_size >= total_io_size:
            break
        
# #  去掉读地址和写地址的前后关系--多线程下没办法顺序执行？        
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

