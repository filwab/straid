import matplotlib.pyplot as plt
import string
import os
import numpy as np

path_md = '../results/MD_trace_results.txt'
path_st = '/home/femu/StRAID/straid/code/results/ST_trace_results.txt'
path_out = '~/StRAID/straid/code/draw/'

tfile = path_st

# 初始化数据列表
band_data = []
write_band_data = []
iops_data = []
latency_data = []
read_latency_data=[]

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
        elif line.startswith('Latancy'):
            latency_data = [int(x) for x in next(f).strip().split()]
        elif line.startswith('Read Latancy CDF'):
            read_latency_data = [int(x) for x in next(f).strip().split()]
        else:
            continue

# 将纵坐标转换为毫秒（ms）单位
latency_cdf_ms = np.array(latency_data) / 1e6  # 从纳秒转换为毫秒
read_latency_cdf_ms = np.array(read_latency_data) / 1e6

# 生成百分位数
percentiles = np.linspace(0, 1, len(latency_cdf_ms))

# 找到90%的索引位置
index_90 = int(0.9 * len(percentiles))

index_995 = int(0.995 * len(percentiles))
index_999 = int(0.999 * len(percentiles))
index_9995 = int(0.9995 * len(percentiles))


# 绘制平滑曲线
plt.figure(figsize=(12, 6))

plt.plot(percentiles[index_90:], latency_cdf_ms[index_90:], label='Latency CDF', linestyle='-', linewidth=2)
plt.plot(percentiles[index_90:len(read_latency_cdf_ms)], read_latency_cdf_ms[index_90:], label='Read Latency CDF', linestyle='-', linewidth=2)

plt.xlim([0.9, 1.0])
plt.xlabel('Percentile')
plt.ylabel('Latency (ms)')
plt.title('Filsever-CDF')
plt.legend()
plt.grid(True)

#在read_latency_cdf_ms中标注995，999，9995的尾延迟值,用箭头指示，并显示其y值
plt.annotate(f'995: {read_latency_cdf_ms[index_995]:.2f} ms', xy=(0.995, read_latency_cdf_ms[index_995]), xytext=(0.99, read_latency_cdf_ms[index_995]+4),
             arrowprops=dict(facecolor='red', arrowstyle='<-'),)
plt.annotate(f'999: {read_latency_cdf_ms[index_999]:.2f} ms', xy=(0.999, read_latency_cdf_ms[index_999]), xytext=(0.985, read_latency_cdf_ms[index_999]),
                arrowprops=dict(facecolor='red',  arrowstyle='<-'),)
plt.annotate(f'9995: {read_latency_cdf_ms[index_9995]:.2f} ms', xy=(0.9995, read_latency_cdf_ms[index_9995]), xytext=(0.985, read_latency_cdf_ms[index_9995]+4),
                arrowprops=dict(facecolor='red',  arrowstyle='<-'),)


plt.savefig(os.path.join(os.getcwd(), 'filsever_ioda_32thre_latency.png'))
# plt.show()


# # 绘制尾延迟分布图  有bug目前，暂待修改
# x = [i / 1000.0 for i in range(1, 1001)] #没计算百分比
# y = latency_data #上面x也没有保证和y是一样的长度，手动不作数
# fig, ax = plt.subplots(figsize=(8, 6))

# plt.plot(x, y, linewidth=2)
# plt.xscale('log')
# plt.xlabel('Tail Latency (s)')
# plt.ylabel('CDF')
# plt.title('Latency CDF')

# plt.tight_layout()
# plt.savefig(os.path.join(os.getcwd(), tfile[-20:-18] +'_latency.png'))
# plt.close()