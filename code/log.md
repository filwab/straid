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

执行filsever_1.log  --179行
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

执行load-cc自定义配置负载  --198行
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