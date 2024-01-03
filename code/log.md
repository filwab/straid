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