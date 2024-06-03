import matplotlib.pyplot as plt
import numpy as np

'''
同一阵列配置不同模式下的不同负载的尾延迟对比
'''
# Your provided data
modes = ['base','steering']
loads = ['hm0', 'mds0', 'rsrch0', 'wdev0']
data = {
    'base': {
        'hm0': [21.69, 23.00, 25.10],
        'mds0': [1.65, 1.83 , 20.01],
        'rsrch0': [2.58 , 23.33 , 26.06],
        'wdev0': [21.85 , 24.55 , 25.97]
    },
    'steering': {
        'hm0': [23.56 , 24.89 , 26.76],
        'mds0': [1.86 , 2.05 , 2.73],
        'rsrch0': [0.42 , 11.96 , 28.45],
        'wdev0': [23.23 , 24.80 , 26.85]
    },
    'IODA': {
        'hm0': [1.57 , 10.16 , 14.12],
        'mds0': [0.94 , 1.24 , 3.81],
        'rsrch0': [2.56 , 2.83 , 20.07],
        'wdev0': [1.82 , 12.27 , 23.17]
    },
    'group_GC': {
        'hm0': [0.63 , 12.93 , 19.92],
        'mds0': [1.31 , 1.43 , 1.77],
        'rsrch0': [0.26 , 0.33 , 8.64],
        'wdev0': [0.56 , 6.23 , 16.33]
    }
}
loads2 = ['fileserver', 'nload_r', 'nload', 'ycsb']
data2 = {
    'base': {
        'fileserver': [ 48.79 , 49.16 , 49.81 ],
        'nload_r': [ 46.86 , 47.36 , 47.98 ],
        'nload': [ 45.62 , 45.74 , 45.94 ],
        'ycsb': [50.05 , 50.36 , 50.95]
    },
    'steering': {
        'fileserver': [ 46.91 , 51.38 , 55.07 ],
        'nload_r':[ 44.99 , 49.48 , 50.04 ],
        'nload': [ 44.80 , 49.92 , 52.13 ],
        'ycsb': [50.22 , 50.54 , 51.18]
    },
    'IODA': {
        'fileserver': [ 4.70 , 15.95 , 87.93],
        'nload_r': [ 4.74 , 42.48 , 87.59 ],
        'nload': [ 42.75 , 86.03 , 132.54  ],
        'ycsb': [27.48 , 53.19 , 61.23]
    },
    'group_GC': {
        'fileserver': [46.06 , 47.41 , 48.95 ],
        'nload_r': [ 37.58 , 39.34 , 41.97 ],
        'nload': [ 46.17 , 46.56 , 47.32 ],
        'ycsb': [ 47.36 , 49.08 , 50.58]
    }

}
percentiles = ['999', '9995', '9999']

# hatches = ['/', '\\\\','/']
# colors = ['#e98909', '#2ca02c', '#ff7f0e']
edge_colors = ['#70ad47', '#99ff99','#d1edf8', '#dbdb8d' ]  # 边缘颜色，对应不同的hatches
hatches = ['/', '\\','//','\\']
colors = ['#1f77b4', '#ff7f0e','#efbf01', '#2ca02c']
# 绘制每个负载的图
for load in loads2:
    base_data = [data2['base'][load][i] for i in range(len(data2['base'][load]))]
    steering_data = [data2['steering'][load][i] for i in range(len(data2['steering'][load]))]
    ioda_data = [data2['IODA'][load][i] for i in range(len(data2['IODA'][load]))] 
    group_GC_data = [data2['group_GC'][load][i] for i in range(len(data2['group_GC'][load]))]
    
    # 绘图
    fig, ax = plt.subplots(figsize=(8, 6))
    bar_width = 0.21
    index = np.arange(len(percentiles))

    bar1 = ax.bar(index, base_data, bar_width,color=colors[0], label='Base',hatch=hatches[0],edgecolor=edge_colors[0])
    bar2 = ax.bar(index + bar_width, steering_data, bar_width,color=colors[1], label='Steering',hatch=hatches[1],edgecolor=edge_colors[1])
    bar3 = ax.bar(index + 2*bar_width, ioda_data, bar_width,color=colors[2], label='IODA',hatch=hatches[2],edgecolor=edge_colors[2])
    bar4 = ax.bar(index + 3*bar_width, group_GC_data, bar_width,color=colors[3], label='Group_GC',hatch=hatches[3],edgecolor=edge_colors[3])
    # bar1 = ax.bar(index, base_data, bar_width,color=colors[0], label='Base')
    # bar2 = ax.bar(index + bar_width, steering_data, bar_width,color=colors[1], label='Steering')
    # bar3 = ax.bar(index + 2*bar_width, group_GC_data, bar_width,color=colors[2], label='Group_GC')

    ax.set_xlabel('Percentile')
    ax.set_ylabel('Tail Latency (ms)')
    ax.set_title('Comparison of Tail Latency at Different Percentiles for ' + load.capitalize())
    ax.set_xticks(index + bar_width / 2)
    ax.set_xticklabels(percentiles)
    ax.legend()
    plt.savefig(str(load)+'_cpr.png')




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

