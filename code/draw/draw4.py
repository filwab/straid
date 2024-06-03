import matplotlib.pyplot as plt
import numpy as np
# 数据字典

data = {
    'hm0.log': {
        'iosizes': [4, 8, 16, 32, 36, 12, 24, 40, 28, 64],
        'counts': [3228790, 219478, 145186, 82653, 53089, 31239, 17399, 16193, 12669, 134893]
    },
    'mds1.log': {
        'iosizes': [4, 8, 16, 24, 32, 60, 12, 56, 20, 64],
        'counts': [157132, 14409, 7651, 4527, 4009, 6889, 6649, 4656, 4068, 1403186]
    },
    'rsrch0.log': {
        'iosizes': [4, 8, 16, 12, 24, 32, 48, 44, 52, 64],
        'counts': [1026633, 169637, 141770, 5470, 2442, 2001, 47409, 3759, 18450, 8354]
    },
    'wdev0.log': {
        'iosizes': [4, 16, 8, 24, 20, 32, 12, 28, 40, 64],
        'counts': [854674, 104883, 70205, 34453, 28623, 5310, 7777, 4303, 1804, 19980]
    }
}

# 遍历数据字典，为每个负载画图
for logfile, logfile_data in data.items():
    plt.figure(figsize=(10, 6))
    x = np.arange(len(logfile_data['iosizes']))  # 创建一个离散的 x 值数组
    plt.bar(x, logfile_data['counts'], color='#4ABEF0', width=0.83)  # 设置柱形宽度为0.8
    plt.xticks(x, logfile_data['iosizes'])  # 设置 x 轴刻度为 iosizes
    plt.xlabel('IOsize (KB)')
    plt.ylabel('counts')
    plt.title(f'{logfile}: IOsize frequency distribution')
    plt.savefig(str(logfile) + '_iosize.png')