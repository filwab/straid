#ifndef STORAGEMOD_H
#define STORAGEMOD_H

#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>
#include <mutex>
#include <shared_mutex>

#include "encode.h"
#include "decode.h"
#include "logfiles.h"
#include "metadata.h"
#include "cache.h"

int log_scheduler(V_DevQue *v_logques);
bool raid_select(uint64_t length);

class StorageMod
{
public:
    int num_Devs;//gql-阵列中盘的总数量
    int num_datas;//gql-数据盘的数量
    int num_paritys;//gql-校验盘的数量

    V_DevFiles *v_stdfiles;

    MetaMod *meta_mod;

public:
    SEncodeMod *s_encodemod;
    SDecodeMod *s_decodemod;

public:
    V_UserQue v_workinque;//gql-用户请求队列向量--Vector of User Queues
    V_BatchQue v_batchque;//gql-做两段提交机制设计的数据结构--Vector of Batch Queues

    vector<vector<DIO_Info>> v_batchque2;/*gql-这是一个二维向量，其中每个元素都是一个vector<DIO_Info>。DIO_Info是一个结构体，用于表示设备IO信息。因此，v_batchque2可以用来存储和管理多个设备IO请求队列。*/
    mutex v_batchquelk[NUM_THREADS];

    // thread v_raid_workers[NUM_WORKERS];

public:
    StorageMod(V_DevFiles *vstdfiles, MetaMod *metamod)
    {
        num_datas = DATACHUNK_NUM;
        num_paritys = PARITYCHUNK_NUM;
        num_Devs = DATACHUNK_NUM + PARITYCHUNK_NUM;

        v_stdfiles = vstdfiles;
        meta_mod = metamod;

        assert(num_Devs == NUM_DEVFILES);

        // Init_Raid(vstdfiles);

        s_encodemod = new SEncodeMod(num_datas, num_paritys, SCHUNK_SIZE, v_stdfiles, meta_mod);
        s_decodemod = new SDecodeMod(num_datas, num_paritys, SCHUNK_SIZE, v_stdfiles, meta_mod);

        for (size_t i = 0; i < NUM_WORKERS; i++)//gql-为各个线程初始化各请求队列向量
        {
            UserQueue *workerque = new UserQueue(i);
            v_workinque.emplace_back(workerque);
            BatchQueue *batcherque = new BatchQueue(i);
            v_batchque.emplace_back(batcherque);

            vector<DIO_Info> queue;
            queue.resize(50);
            v_batchque2.emplace_back(queue);
            // v_raid_workers[i] = thread(&StorageMod::workers_run, this, i); // disable
        }
    }
    ~StorageMod()
    {
        delete s_encodemod;
        delete s_decodemod;
        for (size_t i = 0; i < v_workinque.size(); i++)
        {
            delete v_workinque.at(i);
        }
        for (size_t i = 0; i < v_batchque.size(); i++)
        {
            delete v_batchque.at(i);
        }
    };

    bool Init_Raid(V_DevFiles *v_devfiles); // Initialize RAID space

    void workers_run(int worker_id);

    bool raid_write_q(UIO_Info uio); // User thread put requests into queue
    bool raid_read_q(UIO_Info uio);

    uint64_t raid_write_direct(UIO_Info uio); // direct RAID accesses
    uint64_t raid_write(UIO_Info uio);
    uint64_t raid_read(UIO_Info uio);

    void split_stripe(uint64_t offset, uint64_t length, vector<int> *stripe_id, vector<uint64_t> *stripe_soff, vector<uint64_t> *stripe_len);
    void split_chunk(uint64_t offset, uint64_t length, vector<int> *chunk_pos, vector<uint64_t> *chunk_soff, vector<uint64_t> *chunk_len);
    vector<DIO_Info> split_chunk2dio(UIO_Info uio);

    void split_stripe_aligned(UIO_Info *uio, vector<UIO_Info *> *uios_out);
    void split_schunk(UIO_Info *uio, vector<UIO_Info *> *uios_out);
    void split_lchunk(UIO_Info *uio, vector<UIO_Info *> *uios_out);
    void split_block(UIO_Info *uio, vector<UIO_Info *> *uios_out);
};

#endif