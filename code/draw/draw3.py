import matplotlib.pyplot as plt
import numpy as np

'''
同一阵列配置不同模式下的不同负载的尾延迟对比
'''
# Your provided data
modes = ['base','steering']
loads = ['fileserver', 'nload', 'nload_r', 'ycsb']
data = {
    'base': {
        'fileserver': [35.37, 39.48, 40.79,40.20],
        'nload': [40.84, 44.60, 44.78, 44.10],
        'nload_r': [38.91, 40.16, 40.47, 41.31],
        'ycsb': [47.27, 49.84, 50.22, 50.98]
    },
    'steering': {
        'fileserver': [37.38, 38.71, 39.01,39.56],
        'nload': [42.05, 44.74, 45.92, 46.23],
        'nload_r': [39.49, 41.83, 42.32, 42.95],
        'ycsb': [48.30, 52.06, 53.41, 54.02]
    }
}
percentiles = ['995', '999', '9995', '9999']

hatches = ['/', '\\\\']
colors = ['#e98909', '#2ca02c']
edge_colors = ['#70ad47', '#99ff99']  # 边缘颜色，对应不同的hatches
# 绘制每个负载的图
for load in loads:
    base_data = [data['base'][load][i] for i in range(len(data['base'][load]))]
    steering_data = [data['steering'][load][i] for i in range(len(data['steering'][load]))]
    
    # 绘图
    fig, ax = plt.subplots(figsize=(8, 6))
    bar_width = 0.28
    index = np.arange(len(percentiles))

    bar1 = ax.bar(index, base_data, bar_width,color=colors[0], label='Base',hatch=hatches[0],edgecolor=edge_colors[0])
    bar2 = ax.bar(index + bar_width, steering_data, bar_width,color=colors[1], label='Steering',hatch=hatches[1],edgecolor=edge_colors[1])

    ax.set_xlabel('Percentile')
    ax.set_ylabel('Tail Latency (ms)')
    ax.set_title('Comparison of Tail Latency at Different Percentiles for ' + load.capitalize())
    ax.set_xticks(index + bar_width / 2)
    ax.set_xticklabels(percentiles)
    ax.legend()
    plt.savefig(str(load)+'_stee.png')




# percentiles = ['995', '999', '9995', '9999']
# hatches = ['/', '||', '\\\\']
# colors = ['#1f77b4', '#ff7f0e', '#2ca02c']
# edge_colors = ['#70ad47', '#9999ff', '#99ff99']  # 边缘颜色，对应不同的hatches
# array_configs = list(data.keys())

# # Create a bar chart for each load
# for load in data['4+1'].keys():
#     fig, ax = plt.subplots()
#     for i, percentile in enumerate(percentiles):
#         values = [data[config][load][i] for config in array_configs]
#         ax.bar(np.arange(len(values)) + i * 0.15, values, 0.15,color=colors[i], label=f'{percentile}', hatch=hatches[i],edgecolor=edge_colors[i])
    
#     ax.set_xlabel('Array_Config')
#     ax.set_ylabel('Improve Ratio (%)')
#     ax.set_title(f'Impro Ratio of {load} at every tail latency')
#     ax.set_xticks(np.arange(len(values)) + 0.2)
#     ax.set_xticklabels(array_configs)
#     ax.axhline(0, color='black', lw=0.8)
#     ax.legend()
#     plt.savefig(str(load)+'_cg.png')

