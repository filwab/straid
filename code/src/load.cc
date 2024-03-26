#include <iostream>
#include <fstream>
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
#include "workerl.h"
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

extern vector<uint64_t> IO_LatArray[];
extern vector<uint64_t> ReadIO_LatArray[];//gql-add

atomic_uint64_t SeqW_Offset(0);
atomic_bool wait_array[NUM_THREADS];
atomic_uint64_t pos_array[NUM_THREADS];


const int L_NUM_WORKERS = NUM_THREADS;
atomic_uint64_t L_DATA_WRITTEN(0);
atomic_uint64_t L_DATA_READ(0);
atomic_uint64_t L_IOCOUNT(0);
vector<double> L_IOLAT[L_NUM_WORKERS];

bool LCollector_endflag = false;


void * self_thread_worker(void *worker_info)
{
    cds::threading::Manager::attachThread();

    Worker_Info *info = (Worker_Info *)worker_info;
    UserQueue *userqueue = info->userqueue;
    StorageMod *storagemod = info->storagemod;

    bool iodir = info->io_dir;
    bool is_degr_read = info->is_degr_read;
    int thread_id = info->thread_id;
    uint64_t base_offset = info->offset;
    uint64_t iosize = info->io_size;
    int count = info->count;
    int total_loop = info->loop;

    // printf(" Worker Thread: %d Start, rw:%d,  iosize:%lu,  io_count:%d,  loop:%d\n", thread_id,iodir,iosize,count,total_loop);

    cpu_set_t mask;
    cpu_set_t get;
    CPU_ZERO(&mask);
    CPU_SET(thread_id, &mask);
    if (sched_setaffinity(0, sizeof(cpu_set_t), &mask) == -1)
    {
        printf("warning: could not set CPU affinity, continuing...\n");
        exit(-1);
    }
    // printf("thread:%d band cpu successful",thread_id);

    wait_array[thread_id].store(false);
    pos_array[thread_id].store(0);

    char *workload_buf = info->workload_buf;
    uint64_t workload_len = info->workload_len;

    

    uint64_t total_data = 0;
    tic(10 + thread_id);
    if (iodir == true)
    {
        for (int loop = 0; loop < total_loop; loop++)
        {
            if (SeqW_Offset.load() * iosize > WRITE_ALL)
                SeqW_Offset.store(0);
            for (int i = 0; i < count; i++)
            {
                if (iosize < BLK_SIZE || iosize % BLK_SIZE != 0)
                {
                    iosize += BLK_SIZE - (iosize % BLK_SIZE);
                }

                uint64_t offset = 0;
                if (info->is_rand)//gql-随机或者顺序写设置(使用SeqW_Offset保证顺序 
                {
                    offset = Rand_offset_align(base_offset, (WRITE_ALL/WRITE_THREAD), iosize);
                }
                else
                {
                    // offset = base_offset + (uint64_t)(i * iosize);
                    offset = SeqW_Offset.fetch_add(1) * iosize;
                }
                lat_tic(thread_id);
                UIO_Info iometa(thread_id, true, workload_buf, offset, iosize);
                storagemod->raid_write_direct(iometa);
                L_DATA_WRITTEN.fetch_add(iosize);
                lat_toc(thread_id);

                L_IOCOUNT.fetch_add(1);
                total_data += iosize;
            }
        }
    }
    else
    {
        for (int loop = 0; loop < total_loop; loop++)
        {
            uint64_t read_length = iosize;

            for (int i = 0; i < count; i++)
            {
                if (!is_degr_read)
                {
                    if (iosize < BLK_SIZE || iosize % BLK_SIZE != 0)
                    {
                        iosize += BLK_SIZE - (iosize % BLK_SIZE);
                    }

                    uint64_t offset = 0;
                    if (info->is_rand)
                    {
                        offset = Rand_offset_align(base_offset, (READ_ALL/READ_THREAD), iosize);
                    }
                    else
                    {
                        offset = base_offset + (uint64_t)(i * iosize);
                    }
                    lat_tic(thread_id);
                    UIO_Info iometa(thread_id, false, workload_buf, offset, iosize);
                    uint64_t ret = storagemod->raid_read(iometa);
                    L_DATA_READ.fetch_add(iosize);
                    uint64_t rtime =lat_toc(thread_id);
                    ReadIO_LatArray[thread_id].emplace_back(rtime);

                }
                
                total_data += iosize;
                L_IOCOUNT.fetch_add(1);
            }
        }
    }

    double time = toc(10 + thread_id);
    // printf("Thread: %d End | Time used: %.2f ms | Total: %ld MB\n", thread_id, (time / 1000 / 1000), (total_data / 1024 / 1024));

    return NULL;
}

void collector_run(LTTest_Item *citem)
{
    uint64_t last_written = L_DATA_WRITTEN.load();
    uint64_t last_read = L_DATA_READ.load();
    uint64_t last_iocnt = L_IOCOUNT.load();
    while (!LCollector_endflag)
    {
        sleep(1);
        uint64_t now_written = L_DATA_WRITTEN.load();
        uint64_t now_read = L_DATA_READ.load();
        uint64_t now_iocnt = L_IOCOUNT.load();

        citem->WBand_persec.emplace_back(now_written - last_written);
        citem->RBand_persec.emplace_back(now_read - last_read);
        citem->Band_persec.emplace_back(now_written + now_read - last_written - last_read);
        citem->IOPS_persec.emplace_back(now_iocnt - last_iocnt);

        last_written = now_written;
        last_read = now_read;
        last_iocnt = now_iocnt;

        // cout << now_written << " " << now_read << " " << now_iocnt << endl;
    }

    vector<uint64_t> *temp = merge_IOLat(L_NUM_WORKERS);//gql-将多个线程的延迟数据合并成一个排序后的vector
    citem->all_IOLat_list.insert(citem->all_IOLat_list.end(), temp->begin(), temp->end());
    vector<uint64_t> *rtemp = merge_ReadIOLat(L_NUM_WORKERS);//gql-将多个线程的读请求延迟数据合并成一个排序后的vector
    citem->read_IOLat_list.insert(citem->read_IOLat_list.end(), rtemp->begin(), rtemp->end());

    citem->all_data_written = L_DATA_WRITTEN.load();
    citem->all_data_read = L_DATA_READ.load();
    citem->all_iocount = L_IOCOUNT.load();
    return;
}

uint64_t Rand_offset_align(uint64_t baseoff, uint64_t max_offset, uint64_t align)
{
    uint64_t ret = 0;
    uint64_t max = max_offset / align - 1;
    ret = (rand() % max) * align + baseoff;
    if (ret > USER_SPACE_LEN)
    {
        uint64_t max_aligned_offsets = (USER_SPACE_LEN - baseoff) / align;
        ret = (rand() % max_aligned_offsets) * align + baseoff;
    }
    if ((ret % align) != 0)
    {
        ret = ret - (ret % (align)) + align;
    }
    assert(ret <= USER_SPACE_LEN);
    return ret;
}

bool offsetjudge(uint64_t off)
{
    uint64_t devoffset = user2dev(off , NULL , NULL);

    int devst=devoff2stripe(devoffset);
    int usrst=user2stripe(off);
    int maxstripe = (USER_SPACE_LEN/SSTRIPE_DATASIZE);

    if (devst > maxstripe || usrst > maxstripe)
    {
        cout<<"Offset Wrong usroff:  " << off <<"  devoff:  "<< devoffset << " devst: "<<devst<<" usrst: "<<usrst << endl;  
        return true;
    }
    return false;
}


int main(int argc, char *argv[])
{

    ofstream outfile;
    outfile.open("./results/Self_workload_results.txt", ios::out | ios::app);
    if (!outfile.is_open())
    {
        cout << "outfile open error" << endl;
        exit(-1);
    }

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
        DevFile *devfile = new DevFile(i, v_fileset[i], v_logfd[i], 0, USER_IO_SPACE);
        v_stdFiles.emplace_back(devfile);
    }

    uint64_t user_start_offset = 0;
    uint64_t user_end_offset = USER_IO_SPACE;

    MetaMod metamod(user_start_offset, user_end_offset, &v_stdFiles);
    StorageMod storagemod(&v_stdFiles, &metamod);
    GloStor = &storagemod;
    
    printf("Generating buffer for Read load & Write load .\n");
    vector<char *> write_buf;
    vector<char *> read_buf;
    for (size_t i = 0; i < WRITE_THREAD; i++)
    {
        char *buf;
        int ret = posix_memalign((void **)&buf, ALIGN_SIZE, 100 * MB);
        assert(ret == 0);
        int init_int = rand() % 0xff;
        memset(buf, init_int, 100 * MB);
        // cout << "write init buffer: " << hex << init_int << endl;
        write_buf.emplace_back(buf);
    }
    for (size_t i = 0; i < READ_THREAD; i++)
    {
        char *buf2;
        int ret = posix_memalign((void **)&buf2, ALIGN_SIZE, 100 * MB);
        assert(ret == 0);
        // cout << "read init buffer: " << hex << init_int << endl;
        read_buf.emplace_back(buf2);
    }

    cout << dec << ">> START multi-worker with " << WRITE_THREAD << " write workers" << endl;
    cout << dec << ">> START multi-worker with " << READ_THREAD << " read workers" << endl;

    tic(9);
    
    vector<Worker_Info *> V_workinfo;
    pthread_t wtids[WRITE_THREAD];
#if WRITE_THREAD > 0
    for (size_t i = 0; i < WRITE_THREAD; i++)
    {
        Worker_Info *info = (Worker_Info *)malloc(sizeof(Worker_Info));
        info->io_dir = true; // Write IO
        info->is_rand = WRITE_MOD;
        info->thread_id = i;
        info->io_size = WRITE_IOSIZE;
        info->offset = o_align(i * (USER_IO_SPACE / WRITE_THREAD), WRITE_IOSIZE);
        info->count = ( WRITE_ALL / WRITE_IOSIZE / WRITE_THREAD );
        info->loop = LOOP;
        info->storagemod = &storagemod;
        info->workload_buf = write_buf.at(i);
        info->workload_len =  (WRITE_ALL / WRITE_THREAD) ;

        int ret = pthread_create(&wtids[i], NULL, self_thread_worker, info);
        if (ret != 0)
        {
            printf("pthread_create error: error_code=%d\n", ret);
        }
        V_workinfo.emplace_back(info);
    }
    cout << dec << "write workload generate complete" << endl;
#endif

    pthread_t rtids[READ_THREAD];
#if READ_THREAD > 0
    for (size_t i = 0; i < READ_THREAD; i++)
    {
        Worker_Info *info = (Worker_Info *)malloc(sizeof(Worker_Info));
        info->io_dir = false; // READ IO
        info->is_rand = READ_MOD;
        info->thread_id = WRITE_THREAD + i;//保证线程的id顺序
        info->io_size = READ_IOSIZE;
        info->offset = o_align(i * (USER_IO_SPACE / READ_THREAD), READ_IOSIZE);
        info->count = ( READ_ALL / READ_IOSIZE / READ_THREAD );
        info->loop = LOOP;
        info->storagemod = &storagemod;
        info->workload_buf = read_buf.at(i);
        info->workload_len =  (READ_ALL / READ_THREAD) ;

        int ret = pthread_create(&rtids[i], NULL, self_thread_worker, info);
        if (ret != 0)
        {
            printf("pthread_create error: error_code=%d\n", ret);
        }
        V_workinfo.emplace_back(info);
    }
    cout << dec << "read workload generate complete" << endl;
#endif
    LTTest_Item this_citem;
    thread collector_load;
    collector_load = thread(collector_run, &this_citem);

    for (size_t i = 0; i < WRITE_THREAD; i++)
    {
        pthread_join(wtids[i], NULL);
    }
    for (size_t i = 0; i < READ_THREAD; i++)
    {
        pthread_join(rtids[i], NULL);
    }
    
    LCollector_endflag=true;
    collector_load.join();

    double timer = toc(9);
    printf("MIX READ & WRITE LOAD END,time-used:%.2f ms\n",timer/1000000);



    /*gql-打印输出信息等*/
    printf("Printing Results\n");
    outfile << "MIX READ & WRITE LOAD" << endl;

    outfile << "Band Persec: "
                << endl;
    for (size_t i = 0; i < this_citem.Band_persec.size() / 1; i++)
    {
        outfile << this_citem.Band_persec.at(i) / 1024 / 1024 << "\t";
    }
    outfile << endl;


    outfile << "Write Band Persec: "
                << endl;
    for (size_t i = 0; i < this_citem.WBand_persec.size() / 1; i++)
    {
        outfile << this_citem.WBand_persec.at(i) / 1024 / 1024 << "\t";
    }
    outfile << endl;


    outfile << "READ Band Persec: "
            << endl;
    for (size_t i = 0; i < this_citem.RBand_persec.size() / 1; i++)
    {
        outfile << this_citem.RBand_persec.at(i) / 1024 / 1024 << "\t";
    }
    outfile << endl;
    
        
    outfile << "IOPS Persec: "
            << endl;
    for (size_t i = 0; i < this_citem.IOPS_persec.size() / 1; i++)
    {
        outfile << this_citem.IOPS_persec.at(i) << "\t";
    }
    outfile << endl;


    outfile << "Latancy CDF: " << endl;
    for (size_t i = 0; i < 2000; i++)
    {
        float percent = (float)(i * 0.0005);
        uint64_t pos = percent * this_citem.all_IOLat_list.size();
        outfile << this_citem.all_IOLat_list.at(pos) << "\t";
    }
    outfile << endl;


    outfile << "Read Latancy CDF: " << endl;//gql-add

#if READ_THREAD > 0
    fflush(stdout);
    for (size_t i = 0; i < 2000; i++)
    {
        float percent = (float)(i * 0.0005);
        uint64_t pos = percent * this_citem.read_IOLat_list.size();
        outfile << this_citem.read_IOLat_list.at(pos) << "\t";
    }
    outfile << endl;
#endif
    outfile << endl;


    /*gql-清除分配空间和一些其他数据*/
    
    for (char* buf : write_buf) 
    {
        free(buf);
    }
    write_buf.clear();

    for (char* buf : read_buf) 
    {
        free(buf);
    }
    read_buf.clear();

    for (Worker_Info* info : V_workinfo) 
    {
        free(info);
    }
    V_workinfo.clear();


    All_Write_Data.store(0);
    All_Read_Data.store(0);
    Batch_Count.store(0);
    Block_Count.store(0);

    L_DATA_WRITTEN.store(0);
    L_DATA_READ.store(0);
    L_IOCOUNT.store(0);
    for (size_t i = 0; i < L_NUM_WORKERS; i++)
    {
        IO_LatArray[i].clear();
        ReadIO_LatArray[i].clear();//gql-add
    }
    LCollector_endflag = false;

    printf("Workload Test END\n");


    sleep(1);
    exit(0);
    return 0;
}
/***

    printf("Generating Workloads for mix Read & Write\n");
    vector<char *> workloadw_buf;
    for (size_t i = 0; i < NUM_THREADS; i++)
    {
        char *buf;
        int ret = posix_memalign((void **)&buf, ALIGN_SIZE, 100 * MB);
        assert(ret == 0);
        int init_int = rand() % 0xff;
        memset(buf, init_int, 100 * MB);
        // cout << "Workload init buffer: " << hex << init_int << endl;
        workloadw_buf.emplace_back(buf);
    }

    cout << dec << ">> START multi-worker with " << NUM_THREADS << " rw workers" << endl;

    {
        printf(">>> Cocurrent mix Read & Write Load\n");
        size_t write_thread_count = NUM_THREADS * WRITE_RATIO; // WRITE_thread
        size_t read_thread_count = NUM_THREADS - write_thread_count; // read_thread

        printf("write thread:%zu,\n",write_thread_count);


        tic(9);
        pthread_t wtids[NUM_THREADS];
        for (size_t i = 0; i < NUM_THREADS; i++)
        {
            Worker_Info *info = (Worker_Info *)malloc(sizeof(Worker_Info));
            if (i < write_thread_count) // generate write io therad
            {
                info->io_dir = true; // Write IO
                info->is_rand = true;
                info->thread_id = i;
                info->io_size = WRITE_IOSIZE;
                info->offset = o_align(i * (WRITE_ALL / write_thread_count), WRITE_IOSIZE);
                info->count = ( WRITE_ALL / WRITE_IOSIZE / write_thread_count );
                info->loop = LOOP;
                info->storagemod = &storagemod;
                info->workload_buf = workloadw_buf.at(i);
                info->workload_len =  (WRITE_ALL / write_thread_count) ;

                int ret = pthread_create(&wtids[i], NULL, self_thread_worker, info);
                if (ret != 0)
                {
                    printf("pthread_create error: error_code=%d\n", ret);
                }
            }
            else // generate read io therad 
            {
                info->io_dir = false; // read IO
                info->is_rand = true;
                info->thread_id = i;
                info->io_size = READ_IOSIZE;
                info->offset = o_align((i-write_thread_count) * (READ_ALL / read_thread_count ), READ_IOSIZE);//gql-问题，读写范围不对称
                info->count = (READ_ALL / READ_IOSIZE / read_thread_count );
                info->loop = LOOP;
                info->storagemod = &storagemod;

                info->workload_buf = workloadw_buf.at(i);
                info->workload_len = (READ_ALL / read_thread_count);

                int ret = pthread_create(&wtids[i], NULL, self_thread_worker, info);
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
         int io_num=(READ_ALL / READ_IOSIZE) + (WRITE_ALL / WRITE_IOSIZE);
        // long long iodata = (READ_DATA)*read_thread_count + WRITE_DATA*write_thread_count;
        // print_throughtput(iodata * LOOP, LOOP * (io_num), timer, "MIX READ & WRITE LOAD END");
        print_throughtput((WRITE_ALL + READ_ALL)* LOOP, LOOP * (io_num), timer, "MIX READ & WRITE LOAD END");
        All_Write_Data.store(0);
        All_Read_Data.store(0);
        Batch_Count.store(0);
        Block_Count.store(0);

    }

***/