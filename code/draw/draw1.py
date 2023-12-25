import matplotlib.pyplot as plt
import string
import os

path_md = '../results/MD_trace_results.txt'
path_st = '../results/ST_trace_results.txt'
path_out = '~/StRAID/straid/code/draw/'

tfile = path_st

# 初始化数据列表
band_data = []
write_band_data = []
iops_data = []
latency_data = []

# 读取数据文件并按行处理数据
with open( tfile , 'r') as f:
    for line in f:
        line = line.strip()
        if line == 'Band Persec:':
            band_data = [int(x) for x in next(f).strip().split()]
        elif line == 'Write Band Persec:':
            write_band_data = [int(x) for x in next(f).strip().split()]
        elif line == 'IOPS Persec:':
            iops_data = [int(x) for x in next(f).strip().split()]
        elif line.startswith('Latency'):
            latency_data = [int(x) for x in next(f).strip().split()]
        else:
            continue

# 绘制带宽、写带宽和 IOPS 变化图
fig, axs = plt.subplots(3, 1, figsize=(12, 8))

axs[0].plot(band_data)
axs[0].set_title('Band Persec')
axs[0].set_xlabel('Time (s)')
axs[0].set_ylabel('Bandwidth (MB/s)')

axs[1].plot(write_band_data)
axs[1].set_title('Write Band Persec')
axs[1].set_xlabel('Time (s)')
axs[1].set_ylabel('Bandwidth (KB/s)')

axs[2].plot(iops_data)
axs[2].set_title('IOPS Persec')
axs[2].set_xlabel('Time (s)')
axs[2].set_ylabel('IOPS')

plt.tight_layout()
plt.savefig(os.path.join(os.getcwd(), 'ioda_32_band.png'))
plt.close()


# # 绘制尾延迟分布图  有bug目前，暂待修改
# x = [i / 1000.0 for i in range(1, 1001)]
# y = latency_data
# fig, ax = plt.subplots(figsize=(8, 6))

# plt.plot(x, y, linewidth=2)
# plt.xscale('log')
# plt.xlabel('Tail Latency (s)')
# plt.ylabel('CDF')
# plt.title('Latency CDF')

# plt.tight_layout()
# plt.savefig(os.path.join(os.getcwd(), tfile[-20:-18] +'_latency.png'))
# plt.close()