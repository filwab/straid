import matplotlib.pyplot as plt
import numpy as np

'''
在不同阵列宽度场景下的不同负载的尾延迟提升效果的变化绘图
输入： 不同负载的不同位尾延迟提升比率

输出： 改进效果的变化情况
'''
# Your provided data
data = {
    '4+1': {
        'fileserver': [90.56, 87.45, 92.56],
        'nload': [92.38, 88.78, 82.91],
        'nload_r': [94.55, 91.25, 90.14],
        'ycsb': [96.34, 92.57, 90.28]
    },
    '6+1': {
        'fileserver': [76.06, 53.19, 10.42],
        'nload': [42.73, 5.61, -5.34],
        'nload_r': [20.08, 28.01, 10.54],
        'ycsb': [37.89, 16.60, 28.40]
    },
    '8+1': {
        'fileserver': [23.01, 11.14, 7.51],
        'nload': [11.50, -93.44, -100.14],
        'nload_r': [26.53, 11.29, -25.01],
        'ycsb': [-5.95, -13.99, -19.09]
    }
}

percentiles = ['995', '999', '9995']
hatches = ['/', '||', '\\\\']
colors = ['#1f77b4', '#ff7f0e', '#2ca02c']
edge_colors = ['#70ad47', '#9999ff', '#99ff99']  # 边缘颜色，对应不同的hatches
array_configs = list(data.keys())

# Create a bar chart for each load
for load in data['4+1'].keys():
    fig, ax = plt.subplots()
    for i, percentile in enumerate(percentiles):
        values = [data[config][load][i] for config in array_configs]
        ax.bar(np.arange(len(values)) + i * 0.15, values, 0.15,color=colors[i], label=f'{percentile}', hatch=hatches[i],edgecolor=edge_colors[i])
    
    ax.set_xlabel('Array_Config')
    ax.set_ylabel('Improve Ratio (%)')
    ax.set_title(f'Impro Ratio of {load} at every tail latency')
    ax.set_xticks(np.arange(len(values)) + 0.2)
    ax.set_xticklabels(array_configs)
    ax.axhline(0, color='black', lw=0.8)
    ax.legend()
    plt.savefig(str(load)+'_cg.png')

# # 提供的数据
# arrays = ['995', '999', '9995']
# load_types = ['fileserver', 'nload', 'nload_r', 'ycsb']
# improvements = [
#     [90.56, 87.45, 85.57],
#     [92.38, 88.78, 82.91],
#     [94.55, 91.25, 90.14],
#     [96.34, 92.57, 90.28],
#     [88.06, 85.19, 83.42],
#     [92.73, 85.61, -5.34],
#     [90.08, 86.01, 83.54],
#     [61.89, 93.60, 91.40],
#     [90.01, 87.14, 85.51],
#     [90.50, -93.44, -100.14],
#     [86.53, 77.29, -25.01],
#     [60.95, 92.99, 90.09]
# ]

# # 创建条形图
# fig, ax = plt.subplots(figsize=(10, 6))

# # 设置数据
# index = np.arange(len(arrays))
# bar_width = 0.2
# opacity = 0.8

# # 绘制条形图
# for i, load_type in enumerate(load_types):
#     improvements_subset = improvements[i*3:(i+1)*3]  # 获取每种load_type对应的改进值
#     bars = plt.bar(index + i * bar_width, improvements_subset, bar_width,
#                    alpha=opacity, label=load_type)

#     # 根据正负值为条形着色
#     for bar, value in zip(bars, improvements_subset):
#         if value < 0:
#             bar.set_color('r')  # 负值使用红色
#         else:
#             bar.set_color('g')  # 正值使用绿色

# plt.xlabel('Array')
# plt.ylabel('Improvement (%)')
# plt.title('Improvement of Tail Latency by Array')
# plt.xticks(index + bar_width * 1.5, arrays)
# plt.legend()

# plt.tight_layout()
# plt.savefig('change.png')
