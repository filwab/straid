import matplotlib.pyplot as plt
import string
import os
import numpy as np

path_md = '../results/MD_trace_results.txt'
path_st = '../results/compare.txt'
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
        elif line.startswith('IODA Read Latancy'):
            ioda_latency_data = [int(x) for x in next(f).strip().split()]
        elif line.startswith('Read Latancy CDF'):
            read_latency_data = [int(x) for x in next(f).strip().split()]
        else:
            continue

# 将纵坐标转换为毫秒（ms）单位
# latency_cdf_ms = np.array(latency_data) / 1e6  # 从纳秒转换为毫秒
ioda_latency_data_ms = np.array(ioda_latency_data) / 1e6 
read_latency_cdf_ms = np.array(read_latency_data) / 1e6

# 生成百分位数
percentiles = np.linspace(0, 1, len(ioda_latency_data_ms))

# 尾延迟数据99、995、999、9995、9999分位数,(延时数据为90以后的数据时候的计算方法)
index_99 = int(0.9 * len(percentiles))
index_995 = int(0.95 * len(percentiles))
index_999 = int(0.99 * len(percentiles))
index_9995 = int(0.995 * len(percentiles))
index_9999 = int(0.999 * len(percentiles))


# 绘制平滑曲线
plt.figure(figsize=(12, 6))

# plt.plot(percentiles[index_90:], latency_cdf_ms[index_90:], label='Latency CDF', linestyle='-', linewidth=2)
plt.plot(percentiles, ioda_latency_data_ms, label='IODA Tail-Latency CDF', linestyle='-', linewidth=2)
plt.plot(percentiles, read_latency_cdf_ms, label='Origin Tail-Latency CDF', linestyle='-', linewidth=2)

plt.xlim([0.9, 1.0])
plt.xlabel('Percentile')
plt.ylabel('Latency (ms)')
plt.title('Filsever-CDF')
plt.legend()
plt.grid(True)

#在read_latency_cdf_ms中标注995，999，9995的尾延迟值,用箭头指示，并显示其y值
plt.annotate(f'995: {read_latency_cdf_ms[index_995]:.2f} ms', xy=(0.95, read_latency_cdf_ms[index_995]), xytext=(0.95, read_latency_cdf_ms[index_995]+2),
             arrowprops=dict(facecolor='red', arrowstyle='<-'),)
plt.annotate(f'999: {read_latency_cdf_ms[index_999]:.2f} ms', xy=(0.99, read_latency_cdf_ms[index_999]), xytext=(0.99, read_latency_cdf_ms[index_999]+2),
                arrowprops=dict(facecolor='red',  arrowstyle='<-'),)
plt.annotate(f'9995: {read_latency_cdf_ms[index_9995]:.2f} ms', xy=(0.995, read_latency_cdf_ms[index_9995]), xytext=(0.995, read_latency_cdf_ms[index_9995]+2),
                arrowprops=dict(facecolor='red',  arrowstyle='<-'),)
#在ioda_latency_data_ms中标注995，999，9995的尾延迟值,用箭头指示，并显示其y值
plt.annotate(f'995: {ioda_latency_data_ms[index_995]:.2f} ms', xy=(0.95, ioda_latency_data_ms[index_995]), xytext=(0.95, ioda_latency_data_ms[index_995]+2),
             arrowprops=dict(facecolor='red', arrowstyle='<-'),)
plt.annotate(f'999: {ioda_latency_data_ms[index_999]:.2f} ms', xy=(0.99, ioda_latency_data_ms[index_999]), xytext=(0.99, ioda_latency_data_ms[index_999]+2),
                arrowprops=dict(facecolor='red',  arrowstyle='<-'),)
plt.annotate(f'9995: {ioda_latency_data_ms[index_9995]:.2f} ms', xy=(0.995, ioda_latency_data_ms[index_9995]), xytext=(0.995, ioda_latency_data_ms[index_9995]+2),
                arrowprops=dict(facecolor='red',  arrowstyle='<-'),)


plt.savefig(os.path.join(os.getcwd(), 'compare.png'))