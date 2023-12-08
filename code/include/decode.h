#ifndef DECODE_H
#define DECODE_H

#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>
#include <atomic>
#include <string.h>
#include <liburing.h>

#include "logfiles.h"
#include "queues.h"
#include "concurrentqueue.h"
#include "define.h"
#include "ecEncoder.h"
#include "metadata.h"

using namespace std;

class SDecodeMod
{
public:
    MetaMod *GloMeta;

public:
    const static int Max_ECthreads = NUM_WORKERS; 
    atomic_bool ecthreadbuf_flag[Max_ECthreads];  

    
    vector<ecEncoder *> v_ecDecoder; //gql-编码模块实例(每个线程一个)
    vector<vector<char *>> v_dedatabuf;//gql-读处理模块数据缓冲区(每个线程一个)
    vector<vector<char *>> v_deparitybuf;//gql-读处理模块校验缓冲区(每个线程一个)   

    int valid;         
    int check;         
    size_t chunk_size; 

public:
    V_DevFiles *v_stdfiles;

public:
    io_uring stdring[NUM_WORKERS];     //gql-每个线程的环形队列实例        
    atomic_uint64_t ring_pending[NUM_WORKERS]; //gql-每个线程的环形队列中的挂起请求数量？？？

public:
    
    SDecodeMod(int valid_in, int check_in, size_t chunksize, V_DevFiles *v_logfiles_in, MetaMod *Meta)
    {
        
        GloMeta = Meta;
        valid = valid_in;
        check = check_in;
        chunk_size = chunksize;
        v_stdfiles = v_logfiles_in;

        
        vector<char *> data_buf;
        for (int nlog = 0; nlog < valid; nlog++)//gql-初始化读处理模块器中每个线程的有效数据盘的数据缓冲区
        {
            char *ptr = NULL;
            int ret = posix_memalign((void **)&ptr, ALIGN_SIZE, SCHUNK_SIZE);
            memset(ptr, 0x03, SCHUNK_SIZE);
            data_buf.emplace_back(ptr);
        }
        v_dedatabuf.emplace_back(data_buf);

        vector<char *> parity_buf;//gql-初始化读处理模块器中每个线程的校验盘的缓冲区
        for (int nlog = 0; nlog < check; nlog++)
        {
            char *ptr = NULL;
            int ret = posix_memalign((void **)&ptr, ALIGN_SIZE, SCHUNK_SIZE);
            memset(ptr, 0x04, SCHUNK_SIZE);
            parity_buf.emplace_back(ptr);
        }
        v_deparitybuf.emplace_back(parity_buf);

        
        ecEncoder *decoder = new ecEncoder(valid, check, chunk_size);//gql-初始化读处理模块器中每个线程的ecEncoder的实例
        v_ecDecoder.emplace_back(decoder);

        
        for (size_t i = 0; i < NUM_WORKERS; i++)
        {
            io_uring_queue_init(RING_QD, &stdring[i], RING_FLAG);//初始化读处理模块器中各线程一个io_uring实例
        }
    };

    ~SDecodeMod()
    {
    }

    uint64_t s_norRead(int thread_id, vector<DIO_Info> v_uios);
    uint64_t s_degRead(int thread_id, vector<char *> *vdatabuf, vector<uint64_t> *vdataoff, vector<int> *vdatapos, vector<uint64_t> *vdatalen);
};

#endif