import pandas as pd
from collections import deque
import os
import numpy as np
import random
import math as mt
# 定义文件路径变量
file_path = '../Traces/8ssd_trace/aliTrace/'  # 请将这里替换为你的文件路径
# 读取CSV文件


def load_data(file_path):
    df = pd.read_csv(file_path, sep='\t', header=None, names=['Type', 'Offset', 'Size'])
    return df[df['Type'] == 'R']  # 过滤出读请求

# 定义统计IO次数的函数
def count_io(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()
        total_lines = len(lines)

        read_io_count = 0
        write_io_count = 0

        for line in lines:
            if 'R' in line:  # 假设读IO标识为'R'
                read_io_count += 1
            elif 'W' in line:  # 假设写IO标识为'W'
                write_io_count += 1

        return total_lines, read_io_count, write_io_count

class LRUNode:
    def __init__(self, key=None):
        self.key = key
        self.prev = None
        self.next = None

class LRUCache:
    def __init__(self, capacity):
        self.capacity = capacity
        self.cache = {}
        self.head = None
        self.tail = None

    def _add_node(self, node):
        node.prev = None
        node.next = self.head
        if self.head!=None:
            self.head.prev = node
        else:
            self.tail = node
        self.head = node
        self.cache[node.key] = node


    def _remove_node(self):
        victim_node = self.tail
        if self.head.next == None:
            self.head = None
            self.tail = None
        else:
            self.tail = self.tail.prev
            self.tail.next = None
        victim_node.prev = None
        victim_node.next = None
        del self.cache[victim_node.key]
        victim_node = None

    
    def _move_to_head(self, node):
        if self.head != node:
            if self.tail == node:
                node.prev.next = None
                self.tail = node.prev
            else:
                node.prev.next = node.next
                node.next.prev = node.prev
            node.next = self.head
            self.head.prev = node
            node.prev = None
            self.head = node

    def get(self, key):
        if key in self.cache:
            node = self.cache[key]
            return node
        else:
            return None

    def put(self, value):
        if len(self.cache) >= self.capacity:
            self._remove_node()
        new_node = LRUNode(key=value)
        self._add_node(new_node)
        self.cache[value] = new_node

# 定义LRU-2缓存类
# 这段代码已经实现了LRU-2缓存算法，以下是对其稍作修改以符合LRU-2算法的要求：

class LRU2Cache:
    def __init__(self, capacity):
        self.capacity = capacity
        self.cache = {}
        self.history = LRUCache(1024)
        self.lru = LRUCache(capacity)

    def access(self, page_id):
        hit = False

        # 检查是否在缓存中
        if page_id in self.cache:
            hit = True
            node = self.lru.get(page_id)
            if node:
                self.lru._move_to_head(node)
        else:
            # 如果不在缓存中，先检查历史记录
            node = self.history.get(page_id)
            if node:
                self.lru.put(page_id)
                self.cache[page_id] = 1
            else:
                self.history.put(page_id)
        return hit

# 定义计算缓存命中率的函数

def calculate_cache_hit_rate(file_path, cache_size_mb=100, page_size_kb=4):
    PAGE_SIZE = page_size_kb * 1024  # 转换为字节
    CACHE_SIZE = cache_size_mb * 1024 * 1024  # 转换为字节
    NUM_PAGES = CACHE_SIZE // PAGE_SIZE  # 计算缓存可以容纳的页数

    # 加载数据
    read_requests = load_data(file_path)

    # 创建LRU-2缓存实例
    cache = LRU2Cache(NUM_PAGES)

    # 统计命中次数和总请求次数
    hit_count = 0
    total_requests = 0

    sub_page = 0

    # 模拟读请求处理
    for _, row in read_requests.iterrows():
        offset = row['Offset']
        size = row['Size']
        total_requests += 1

        

        start_offset = offset
        end_offset = offset + size

        start_page_id = start_offset // PAGE_SIZE
        end_page_id = end_offset // PAGE_SIZE

        for i in  range(start_page_id, end_page_id + 1):
            if cache.access(i):
                hit_count += 1
            
            sub_page += 1
            

        # nums = size // PAGE_SIZE
        # if size % PAGE_SIZE != 0:
        #     nums += 1         

        # for i in range(nums):
        #     # 计算页号
        #     page_id = (offset + i * PAGE_SIZE) // PAGE_SIZE

        #     # 访问缓存
        #     if cache.access(page_id):
        #         hit_count += 1

        # # 计算页号
        # page_id = offset // PAGE_SIZE

        # # 访问缓存
        # if cache.access(page_id):
        #     hit_count += 1


    # 计算缓存命中率
    hit_rate = hit_count / sub_page if total_requests > 0 else 0

    return {
        'total_read_requests': total_requests,
        'total_sub_page': sub_page,
        'cache_hit_count': hit_count,
        'cache_hit_rate': hit_rate
    }

# files = ["0.csv", "1.csv", "2.csv"]
files = ["64.log","112.log","131.log","188.log","251.log","295.log"]
# files = ["fileserver_1.log"]

# for filename in sorted(os.listdir(file_path), key=lambda x: int(x.split('.')[0])):
for filename in files:
# for filename in files:
    if filename.endswith('.log'):
        print("processing file: ", filename)
        input_file = os.path.join(file_path, filename)
        total_requests, read_requests, write_requests = count_io(input_file)
        # 计算读请求比例
        read_ratio = read_requests / total_requests
        print("read requests:{}, write requests:{},total requests:{},read ratio:{:.2%}".format(read_requests, write_requests, total_requests, read_ratio))
        result = calculate_cache_hit_rate(input_file)
        print(f'Total read requests: {result["total_read_requests"]}')
        print(f'Total sub page: {result["total_sub_page"]}')
        print(f'Cache hit count: {result["cache_hit_count"]}')
        print(f'Cache hit rate: {result["cache_hit_rate"]:.2%}')
        print('---------------------------------')
        print()






