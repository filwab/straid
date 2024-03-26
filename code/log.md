### 1225脚本执行记录
##### 改变尾延迟统计数据后的：非ioda模式执行脚本记录
```txt
Open Trace Files
Init RAID System
Number of SSDs: 5 | Data Chunks: 4 | Parity Chunks: 1
Number of Workers: 32
Open Files
Generating DevFile
Generating MetaMod
Generating StorageMod
Run Trace ./Traces/fileserver_1.log
[Reading] lineCount = 1656237
Testing Trace
[./Traces/fileserver_1.log] | Throughtput: 827.95 MB/s | IOPS: 9039.90 | Time used: 183214.16 ms
[./Traces/fileserver_1.log] | reconstrcut read num: 0 | total read num: 1160005 | 
Printing Results
```
##### 改变尾延迟统计数据后的：ioda模式执行脚本记录
```txt
Open Trace Files
Init RAID System
Number of SSDs: 5 | Data Chunks: 4 | Parity Chunks: 1
Number of Workers: 32
Open Files
Generating DevFile
Generating MetaMod
Generating StorageMod
Run Trace ./Traces/fileserver_1.log
[Reading] lineCount = 1656237
Testing Trace
[./Traces/fileserver_1.log] | Throughtput: 901.80 MB/s | IOPS: 9846.21 | Time used: 168210.70 ms
[./Traces/fileserver_1.log] | reconstrcut read num: 25527 | total read num: 1160102 | 
Printing Results
```

### 0112脚本执行记录

执行自己生成的load_1脚本(在之前的测试warm的基础上)  --158行
```txt
Open Trace Files
Init RAID System
Number of SSDs: 5 | Data Chunks: 4 | Parity Chunks: 1
Number of Workers: 32
Open Files
Generating DevFile
Generating MetaMod
Generating StorageMod
Run Trace ./Traces/load_1.log
[Reading] lineCount = 175046
Testing Trace
[./Traces/load_1.log] | Throughtput: 426.91 MB/s | IOPS: 1305.64 | Time used: 134069.09 ms
[./Traces/load_1.log] | reconstrcut read num: 0 | total read num: 143876 | 
[./Traces/load_1.log] - read tail latency result :
Average latency: | 99.0% latency: | 99.5% latency: | 99.9% latency: | 99.95% latency:
6.93 ms | 48.56 ms | 48.89 ms | 49.60 ms | 49.88 ms
Printing Results

```

执行filsever_1.log  logfile.md --179行
```txt
Open Trace Files
Init RAID System
Number of SSDs: 5 | Data Chunks: 4 | Parity Chunks: 1
Number of Workers: 32
Open Files
Generating DevFile
Generating MetaMod
Generating StorageMod
Run Trace ./Traces/fileserver_1.log
[Reading] lineCount = 1656237
Testing Trace
[./Traces/fileserver_1.log] | Throughtput: 403.18 MB/s | IOPS: 4402.10 | Time used: 376237.62 ms
[./Traces/fileserver_1.log] | reconstrcut read num: 0 | total read num: 1160122 | 
[./Traces/fileserver_1.log] - read tail latency result :
Average latency: | 99.0% latency: | 99.5% latency: | 99.9% latency: | 99.95% latency:
2.75 ms | 48.46 ms | 48.68 ms | 49.16 ms | 49.72 ms
Printing Results
```

执行load-cc自定义配置负载 logfile.md  --198行
```txt
Number of SSDs: 5 | Data Chunks: 4 | Parity Chunks: 1
This System has 64 Processors
Generating buffer for Read load & Write load .
>> START multi-worker with 20 write workers
>> START multi-worker with 12 read workers
write workload generate complete
read workload generate complete
MIX READ & WRITE LOAD END,time-used:93915.22 ms
Printing Results
Workload Test END
```

0115:添加带宽利用率和gc阈值和victim line 阈值后进行测试：

执行load_1 trace测试结果
```txt
Number of SSDs: 5 | Data Chunks: 4 | Parity Chunks: 1
Number of Workers: 32
Open Files
Generating DevFile
Generating MetaMod
Generating StorageMod
Run Trace ./Traces/load_1.log
[Reading] lineCount = 175046
Testing Trace
[./Traces/load_1.log] | Throughtput: 1842.25 MB/s | IOPS: 5634.18 | Time used: 31068.58 ms
[./Traces/load_1.log] | reconstrcut read num: 0 | total read num: 144606 | 
[./Traces/load_1.log] - read tail latency result :
Average latency: | 99.0% latency: | 99.5% latency: | 99.9% latency: | 99.95% latency:
1.85 ms | 8.18 ms | 9.21 ms | 10.92 ms | 11.63 ms
Printing Results
```

重启机子后跑load.cc  logfile.md --235行(性能表现)
```txt
Number of SSDs: 5 | Data Chunks: 4 | Parity Chunks: 1
This System has 64 Processors
Generating buffer for Read load & Write load .
>> START multi-worker with 12 write workers
>> START multi-worker with 20 read workers
write workload generate complete
read workload generate complete
MIX READ & WRITE LOAD END,time-used:23868.55 ms
Printing Results
Workload Test END
```


跑filsever.log --254行
```txt

Number of SSDs: 5 | Data Chunks: 4 | Parity Chunks: 1
Number of Workers: 32
Open Files
Generating DevFile
Generating MetaMod
Generating StorageMod
Run Trace ./Traces/fileserver_1.log
[Reading] lineCount = 1656237
Testing Trace
[./Traces/fileserver_1.log] | Throughtput: 785.01 MB/s | IOPS: 8571.09 | Time used: 193235.22 ms
[./Traces/fileserver_1.log] | reconstrcut read num: 0 | total read num: 1160093 | 
[./Traces/fileserver_1.log] - read tail latency result :
Average latency: | 99.0% latency: | 99.5% latency: | 99.9% latency: | 99.95% latency:
1.51 ms | 36.40 ms | 37.41 ms | 38.95 ms | 39.30 ms
Printing Results
```

load.cc  读64K 写fullsize ：20/12线程
```txt
Number of SSDs: 5 | Data Chunks: 4 | Parity Chunks: 1
This System has 64 Processors
Generating buffer for Read load & Write load .
>> START multi-worker with 12 write workers
>> START multi-worker with 20 read workers
write workload generate complete
read workload generate complete
MIX READ & WRITE LOAD END,time-used:52082.57 ms
Printing Results
Workload Test END
```

load.cc 读写都64K，20/12线程

```txt
Number of SSDs: 5 | Data Chunks: 4 | Parity Chunks: 1
This System has 64 Processors
Generating buffer for Read load & Write load .
>> START multi-worker with 12 write workers
>> START multi-worker with 20 read workers
write workload generate complete
read workload generate complete
MIX READ & WRITE LOAD END,time-used:55110.52 ms
Printing Results
Workload Test END
```

重新生成80Gio负载进行双模式测试尾延迟表现

base情况下：底层已经有GC的情况了
```txt
Number of SSDs: 5 | Data Chunks: 4 | Parity Chunks: 1
Number of Workers: 32
Open Files
Generating DevFile
Generating MetaMod
Generating StorageMod
Run Trace ./Traces/load_80g.log
[Reading] lineCount = 546660
Testing Trace
[./Traces/load_80g.log] | Throughtput: 675.54 MB/s | IOPS: 3330.93 | Time used: 164116.45 ms
[./Traces/load_80g.log] | reconstrcut read num: 0 | total read num: 766499 | 
[./Traces/load_80g.log] - read tail latency result :
Average latency: | 99.0% latency: | 99.5% latency: | 99.9% latency: | 99.95% latency:
5.40 ms | 42.77 ms | 43.04 ms | 43.58 ms | 43.85 ms
Printing Results
```
ioda模式下：

```txt
Init RAID System
Number of SSDs: 5 | Data Chunks: 4 | Parity Chunks: 1
Number of Workers: 32
Open Files
Generating DevFile
Generating MetaMod
Generating StorageMod
Run Trace ./Traces/load_80g.log
[Reading] lineCount = 546660
Testing Trace
[./Traces/load_80g.log] | Throughtput: 1875.90 MB/s | IOPS: 9249.62 | Time used: 59100.79 ms
[./Traces/load_80g.log] | reconstrcut read num: 21086 | total read num: 765788 | 
[./Traces/load_80g.log] - read tail latency result :
Average latency: | 99.0% latency: | 99.5% latency: | 99.9% latency: | 99.95% latency:
0.85 ms | 2.78 ms | 28.24 ms | 31.14 ms | 31.63 ms
```
测试filsever-1.log

ioda 模式下
```txt
Open Trace Files
Init RAID System
Number of SSDs: 5 | Data Chunks: 4 | Parity Chunks: 1
Number of Workers: 32
Open Files
Generating DevFile
Generating MetaMod
Generating StorageMod
Run Trace ./Traces/fileserver_1.log
[Reading] lineCount = 1656237
Testing Trace
[./Traces/fileserver_1.log] | Throughtput: 1231.15 MB/s | IOPS: 13442.18 | Time used: 123211.90 ms
[./Traces/fileserver_1.log] | reconstrcut read num: 27552 | total read num: 1160187 | 
[./Traces/fileserver_1.log] - read tail latency result :
Average latency: | 99.0% latency: | 99.5% latency: | 99.9% latency: | 99.95% latency:
0.34 ms | 1.24 ms | 1.54 ms | 3.02 ms | 27.71 ms
Printing Results
```

base模式下

```txt
Open Trace Files
Init RAID System
Number of SSDs: 5 | Data Chunks: 4 | Parity Chunks: 1
Number of Workers: 32
Open Files
Generating DevFile
Generating MetaMod
Generating StorageMod
Run Trace ./Traces/fileserver_1.log
[Reading] lineCount = 1656237
Testing Trace
[./Traces/fileserver_1.log] | Throughtput: 553.18 MB/s | IOPS: 6039.90 | Time used: 274215.85 ms
[./Traces/fileserver_1.log] | reconstrcut read num: 0 | total read num: 1160034 | 
[./Traces/fileserver_1.log] - read tail latency result :
Average latency: | 99.0% latency: | 99.5% latency: | 99.9% latency: | 99.95% latency:
1.95 ms | 45.18 ms | 45.54 ms | 46.21 ms | 46.54 ms
Printing Results
```


# 0229测试new

测试filesever和load-80g在两种模式下的性能表现。
