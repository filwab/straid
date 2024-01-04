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
#include "concurrentqueue.h"

#include "tools.h"
#include "worker.h"
#include "encode.h"
#include "decode.h"
#include "logfiles.h"
#include "storageMod.h"
#include "define.h"
#include "metadata.h"

using namespace std;

extern atomic_uint64_t All_Write_Data;
extern atomic_uint64_t All_Read_Data;
extern atomic_uint64_t Batch_Count;
extern atomic_uint64_t Block_Count;
extern atomic_uint64_t Cache_Hit;
extern atomic_uint64_t Cache_Miss;
extern atomic_bool proc_end_flag;

extern StorageMod *GloStor;

int main(int argc, char *argv[])
{
    cout << "Init System" << endl;
    assert(NUM_DEV == (DATACHUNK_NUM + PARITYCHUNK_NUM));
    assert(PARITYCHUNK_NUM <= 2);
    assert(DATACHUNK_NUM > 1);
    cout << dec << "Number of SSDs: " << NUM_DEV << " | Data Chunks: " << DATACHUNK_NUM << " | Parity Chunks: " << PARITYCHUNK_NUM << endl;

    srand(time(0));
    DropCaches(3);
    sleep(1);

    int cpus = sysconf(_SC_NPROCESSORS_CONF);
    printf("This System has %d Processors\n", cpus);

    /*init storageMod and do some other inits*/
    vector<string> v_fileset;
    for (int i = 0; i < NUM_DEV; i++)
    {
        string lfile = "/dev/nvme" + to_string(i) + "n1";
        v_fileset.push_back(lfile);
    }
    assert(NUM_DEV <= v_fileset.size());

    vector<int> v_logfd;
    for (size_t i = 0; i < v_fileset.size(); i++)
    {
        int fd = open(v_fileset[i].c_str(), O_RDWR | O_DIRECT | O_TRUNC);
        assert(fd != -1);
        v_logfd.emplace_back(fd);
    }

    V_DevFiles v_stdFiles;
    for (size_t i = 0; i < (NUM_DEVFILES); i++)
    {
        DevFile *devfile = new DevFile(i, v_fileset[i], v_logfd[i], 0, STRA_SPACE_LEN);
        v_stdFiles.emplace_back(devfile);
    }

    uint64_t user_start_offset = 0;
    uint64_t user_end_offset = USER_SPACE_LEN;

    MetaMod metamod(user_start_offset, user_end_offset, &v_stdFiles);
    StorageMod storagemod(&v_stdFiles, &metamod);
    GloStor = &storagemod;
    

    printf("Generating Workloads for mix Read & Write\n");
    vector<char *> workloadw_buf;
    for (size_t i = 0; i < NUM_THREADS; i++)
    {
        char *buf;
        int ret = posix_memalign((void **)&buf, ALIGN_SIZE, (WORKLOAD_DATA / NUM_THREADS) + MB);
        assert(ret == 0);
        int init_int = rand() % 0xff;
        memset(buf, init_int, (WORKLOAD_DATA / NUM_THREADS));
        // cout << "Workload init buffer: " << hex << init_int << endl;
        workloadw_buf.emplace_back(buf);
    }

    cout << dec << ">> START multi-worker with " << NUM_THREADS << " rw workers" << endl;

    {
        printf(">>> Cocurrent mix Read & Write Load\n");
        size_t write_thread_count = NUM_THREADS * WRITE_RATIO; // WRITE_thread
        size_t read_thread_count = NUM_THREADS - write_thread_count; // read_thread


        tic(9);
        pthread_t wtids[NUM_THREADS];
        for (size_t i = 0; i < NUM_THREADS; i++)
        {
            if (i < write_thread_count)/* generate write io therad */
            {
                Worker_Info *info = (Worker_Info *)malloc(sizeof(Worker_Info));
                info->io_dir = true; // Write IO
                info->is_rand = true;
                info->thread_id = i;
                info->io_size = WRITE_IOSIZE;
                info->offset = o_align(i * (WORKLOAD_DATA / write_thread_count), WRITE_IOSIZE);
                info->count = ( WRITE_DATA / WRITE_IOSIZE / write_thread_count);
                info->loop = LOOP;
                info->storagemod = &storagemod;
                info->workload_buf = workloadw_buf.at(i);
                info->workload_len = ( WRITE_DATA / write_thread_count);

                int ret = pthread_create(&wtids[i], NULL, thread_worker, info);
                if (ret != 0)
                {
                    printf("pthread_create error: error_code=%d\n", ret);
                }
            }
            else /* generate read io therad */
            {
                Worker_Info *info = (Worker_Info *)malloc(sizeof(Worker_Info));
                info->io_dir = false; // read IO
                info->is_rand = true;
                info->thread_id = i;
                info->io_size = READ_IOSIZE;
                info->offset = o_align((i-write_thread_count) * (WORKLOAD_DATA / read_thread_count ), READ_IOSIZE);//gql-问题，读写范围不对称
                info->count = (READ_DATA / READ_IOSIZE / read_thread_count);
                info->loop = LOOP;
                info->storagemod = &storagemod;

                info->workload_buf = workloadw_buf.at(i);
                info->workload_len = (READ_DATA / read_thread_count);

                int ret = pthread_create(&wtids[i], NULL, thread_worker, info);
                if (ret != 0)
                {
                    printf("pthread_create error: error_code=%d\n", ret);
                }            
            }
            
        }
        for (size_t i = 0; i < NUM_THREADS; i++)
        {
            pthread_join(wtids[i], NULL);
        }
        double timer = toc(9);
        print_throughtput(WORKLOAD_DATA * LOOP, LOOP * ((READ_DATA / READ_IOSIZE) + (WRITE_DATA / WRITE_IOSIZE)), timer, "MIX READ & WRITE LOAD END");
        All_Write_Data.store(0);
        All_Read_Data.store(0);
        Batch_Count.store(0);
        Block_Count.store(0);

    }
    sleep(1);
    exit(0);
    return 0;
}
