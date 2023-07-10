#ifndef DEFINE_H
#define DEFINE_H

#include <vector>
#include "tbb/concurrent_hash_map.h"
#include "libcuckoo/cuckoohash_map.hh"
#include "concurrentqueue.h"
#include "liburing.h"
#include "kfifo.h"

using namespace std;

#define KB (1024LL)
#define MB (1024 * 1024LL)
#define GB (1024 * 1024 * 1024LL)

#define NUM_DEV (DATACHUNK_NUM + PARITYCHUNK_NUM)            // Total SSD device num
#define SECTOR_SIZE (512)      
#define ALIGN_SIZE SECTOR_SIZE 
#define BLK_SIZE (4 * KB)      // block size in storage devices

#define FULLSIZE (DATACHUNK_NUM * SCHUNK_SIZE)
#define PARTSIZE (SCHUNK_SIZE)
#define DATACHUNK_NUM (4)                              // Number of data chunks // to change 
#define PARITYCHUNK_NUM (1)                            // Number of parity chunks //to change 
#define NUM_DEVFILES (DATACHUNK_NUM + PARITYCHUNK_NUM) 
#define NUM_WORKERS NUM_THREADS                        // Number of RAID worker threads

#define NUM_THREADS (32)
#define DATASET_SIZE (1 * GB)
#define LOOP (1) // workload loop num
#define USER_SPACE_LEN (50 * GB)
// #define IO_SIZE (64 * KB) 
// #define IO_SIZE (PARTSIZE)
#define IO_SIZE (FULLSIZE) // to change 

#define STRA_SPACE_LEN (50 * GB) // RAID Space footprint in a SSD
#define SCHUNK_SIZE (64 * KB)
#define SSTRIPE_SIZE (SCHUNK_SIZE * NUM_DEVFILES)
#define SSTRIPE_DATASIZE (SCHUNK_SIZE * DATACHUNK_NUM)
#define IDLE_QUELEN (4)
#define PCACHE_SIZE (4096 * 16)

#define QUE_MAXLEN (MB)
#define QUE_BLOCK BLK_SIZE

#define RING_QD (32)
#define RING_FLAG (IORING_SETUP_IOPOLL)

typedef unsigned char u8;
#define MMAX 64
#define KMAX 64

struct UIO_Info;
struct DIO_Info;
struct UserQueue;
struct BatchQueue;
struct DevQueue;
struct DevFile;
struct SSTEntry;

typedef tbb::concurrent_hash_map<uint64_t, vector<char *>>::accessor cacheAccessor;
typedef tbb::concurrent_hash_map<uint64_t, vector<char *>>::const_accessor const_cacheAccessor;

typedef moodycamel::ConcurrentQueue<UIO_Info> UsrQue_Item;
typedef moodycamel::ConcurrentQueue<DIO_Info> DevQue_Item;
typedef KQueue LRUQue_Item;
typedef moodycamel::ConcurrentQueue<pair<uint64_t, vector<char *>>> CacheQue_Item;

typedef libcuckoo::cuckoohash_map <uint64_t, SSTEntry *> Table_Hash;//cuckoohash_map

typedef vector<BatchQueue *> V_BatchQue;
typedef vector<UserQueue *> V_UserQue;
typedef vector<io_uring *> V_UserRing;
typedef vector<DevQueue *> V_DevQue;
typedef vector<DevFile *> V_DevFiles;

#endif