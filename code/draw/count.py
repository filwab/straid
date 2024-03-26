import matplotlib.pyplot as plt
import numpy as np

input_path = '../Traces/hmr/'
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


file= 'hm_fh_range.log'
plot_offset_distribution(file)