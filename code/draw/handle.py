import random
import numpy as np
# 配置参数
KB = 1024 
MB = 1024*1024
GB = 1024*1024*1024

RAN = (10 * MB)

offset_range = (0, 50 * GB)  # IO 请求的范围
handle_range = (0, 49 * GB)  # 处理的范围
max_offrange = (49 * GB)
wio_list = [4*KB, 64*KB, 64*2*KB, 80*KB, 96*KB, 128*KB] 
wio_weights = [3, 1, 5, 1, 1, 1 ]

input_path = "../Traces/8ssd_trace/"  # 输入文件路径
output_path = "../Traces/8ssd_trace/10m_range/"  # 输出文件路径

def align_offset_to_64kb(offset):
    align_to = 64 * 4 * 1024  # 64KB*4
    return (offset // align_to) * align_to

def process_log_file(input_file, output_file):

    with open(input_file, 'r') as fin, open(output_file, 'w') as fout:

        off=[]
        for oline in fin:
            oparts = oline.strip().split('\t')
            if len(oparts) != 3:
                print(f"无效的行格式: {line}")
                continue
            off.append(int(oparts[1]))
        off = np.array(off)
        max_offset = np.max(off)

        fin.seek(0)
        for line in fin:
            parts = line.strip().split('\t')
            if len(parts) != 3:
                print(f"无效的行格式: {line}")
                continue
            
            operation, offset, size = parts
            #根据max_offset的大小，等比调整offset的范围到max_offrange内
            offset = int(offset)
            offset = int((offset / max_offset)* max_offrange)

            size = int(size)


            # 在offset前后1GB内重新随机生成一个偏移地址
            final_offset = random.randint(0 if (offset - RAN) < 0 else (offset - RAN), max_offrange if (offset + RAN) > max_offrange else (offset + RAN))
            if operation == 'W':  # 只处理写操作        
                aligned_offset = align_offset_to_64kb(final_offset)
                fout.write(f"{operation}\t{aligned_offset}\t{size}\n")
                pass
            else:  # 读操作直接写入文件不做处理
                if size > 8192:
                    size = int(size/4)//4096*4096
                    size = 4096 if size == 0 else size
                fout.write(f"{operation}\t{offset}\t{size}\n")

def process_origin_file(input_file, output_file):

    with open(input_file, 'r') as fin, open(output_file, 'w') as fout:
        for line in fin:
            parts = line.strip().split()
            if len(parts) != 5:
                print(f"无效的行格式: {line}")
                continue
            
            operation, offset, size = int(parts[4]) , int (parts[2]), int(parts[3])
            
            offset = ((offset*512) // 4096) * 4096
            size = ((size*512) // 4096) * 4096
            size = 4096 if size == 0 else size

            #根据max_offset的大小，等比调整offset的范围到max_offrange内
            ope = "W" if operation == 0 else "R"

            fout.write(f"{ope}\t{offset}\t{size}\n")
# 使用示例
files = ["rsrch0.log"]

for file in files:
    input_file = input_path + file
    output_file = output_path + file
    print(f"处理日志文件: {file}")
    process_log_file(input_file , output_file)

