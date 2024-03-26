#ifndef WORKER_H
#define WORKER_H

#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>

#include <cds/init.h>
#include <cds/threading/model.h>
#include <cds/container/feldman_hashmap_dhp.h>
#include <cds/gc/hp.h>

#include "queues.h"
#include "define.h"
#include "logfiles.h"
#include "metadata.h"
#include "storageMod.h"

using namespace std;
/*gql-: for self-config workload*/

#define WORKLOAD_DATA (4 * GB)
#define WRITE_RATIO (10.0/16) //how much thread for write io 

#define WRITE_IOSIZE (FULLSIZE)
#define READ_IOSIZE (4 * KB)
// #define WRITE_IOSIZE (PARTSIZE)
// #define READ_IOSIZE (PARTSIZE)
#define IO_COUNT (1000000)

#define WRITE_DATA (2 * GB)
#define READ_DATA (2 * GB)
#define USER_IO_SPACE (50 * GB)

#define WRITE_THREAD (1)
#define READ_THREAD (NUM_THREADS-WRITE_THREAD)

#define SEQ_IO (false)
#define RAND_IO (true)

#define WRITE_MOD (RAND_IO)
#define READ_MOD (RAND_IO)

/*for test*/
#define WRITE_ALL (20 * GB)
#define READ_ALL (20 * GB)


struct Worker_Info
{
    int thread_id; // Thread ID

    UserQueue *userqueue;
    StorageMod *storagemod;

    bool io_dir;  // IO direction
    bool is_rand; // is random access
    bool is_degr_read;

    uint64_t io_size; // io size
    int count;        // io conut in a loop
    int loop;         // num of loops

    uint64_t offset;

    char *workload_buf; // pre-assigned workload buf
    uint64_t workload_len;
};

struct LTTest_Item
{
    uint64_t all_data_written;
    uint64_t all_data_read;
    uint64_t all_iocount;

    vector<uint64_t> IOPS_persec;
    vector<uint64_t> Band_persec;
    vector<uint64_t> WBand_persec;
    vector<uint64_t> RBand_persec;

    vector<uint64_t> all_IOLat_list;
    vector<uint64_t> read_IOLat_list;//gql-所有线程read请求的延迟列表
};


uint64_t Rand_offset_align(uint64_t baseoff, uint64_t max_offset, uint64_t align);

/*gql:- thread worker for running self-config workload */
void *self_thread_worker(void *worker_info);
void collector_run(LTTest_Item *citem);
bool offsetjudge(uint64_t off);

#endif