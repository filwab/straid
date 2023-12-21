#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <string>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>
#include <atomic>

#include <cds/init.h>
#include <cds/threading/model.h>
#include <cds/container/feldman_hashmap_dhp.h>
#include <cds/gc/hp.h>

#include "tools.h"
#include "worker.h"
#include "encode.h"
#include "decode.h"
#include "logfiles.h"
#include "storageMod.h"
#include "metadata.h"
#include "split.h"
#include "define.h"

using namespace std;

extern vector<uint64_t> IO_LatArray[];
extern vector<uint64_t> ReadIO_LatArray[];//gql-add

extern atomic_uint64_t All_Write_Data;
extern atomic_uint64_t All_Read_Data;

extern atomic_uint64_t Cache_Hit;
extern atomic_uint64_t Cache_Miss;

extern atomic_uint64_t RECONST_REQ_NUM;
extern atomic_uint64_t TOTAL_READ_NUM;

const int G_NUM_WORKERS = NUM_THREADS;

atomic_uint64_t iodir_cnt(0);
atomic_uint64_t off_cnt(0);
atomic_uint64_t len_cnt(0);

atomic_uint64_t G_DATA_WRITTEN(0);
atomic_uint64_t G_DATA_READ(0);
atomic_uint64_t G_IOCOUNT(0);
vector<double> G_IOLAT[G_NUM_WORKERS];

extern StorageMod *GloStor;
bool Collector_endflag = false;

struct TTest_Item
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

void worker_run(int thread_id,
                StorageMod *storagemod,
                vector<bool> *iodir_l,
                vector<uint64_t> *off_l,
                vector<uint64_t> *len_l,
                // vector<bool>::const_iterator iodir_itr,
                // vector<uint64_t>::const_iterator off_itr,
                // vector<uint64_t>::const_iterator len_itr,
                uint64_t count)
{
    cds::threading::Manager::attachThread();
    char *buf;
    int ret = posix_memalign((void **)&buf, ALIGN_SIZE, 100 * MB);
    assert(ret == 0);

    uint64_t offset = 0;
    uint64_t length = 0;
    bool iodir = 0;

    for (size_t io = 0; io < count; io++)
    {
        offset = off_l->at(off_cnt.fetch_add(1));//gql-多个线程从同一个vector中取出一个值，原子操作保证请求按序被多个线程取下来执行
        length = len_l->at(len_cnt.fetch_add(1));
        iodir = iodir_l->at(iodir_cnt.fetch_add(1));

        // uint64_t offset = *off_itr++;
        // uint64_t length = *len_itr++;
        // bool iodir = *iodir_itr++;

        if (offset > USER_SPACE_LEN)
        {
            offset = offset % USER_SPACE_LEN;
        }
        if ((offset % ALIGN_SIZE) != 0)
        {
            offset = offset - (offset % (ALIGN_SIZE)) + ALIGN_SIZE;
        }
        if ((length % ALIGN_SIZE) != 0)
        {
            length = length - (length % (ALIGN_SIZE)) + ALIGN_SIZE;
        }

        // cout << io << " off: " << offset << "  len: " << length << endl;

        tic(10 + thread_id);
        if (iodir == 0)
        {
            UIO_Info iometa(thread_id, false, buf, offset, length);
            storagemod->raid_read(iometa);
            G_DATA_READ.fetch_add(length);
            double rtime = toc(10 + thread_id);
            ReadIO_LatArray[thread_id].emplace_back(rtime);//记录读请求的延迟
        }
        else
        {
            UIO_Info iometa(thread_id, true, buf, offset, length);
            storagemod->raid_write_direct(iometa);
            G_DATA_WRITTEN.fetch_add(length);
        }
        G_IOCOUNT.fetch_add(1);
        double time = toc(10 + thread_id);
        IO_LatArray[thread_id].emplace_back(time);//记录的是读写的所有请求的延迟
    }
    return;
}

void collector_run(TTest_Item *citem)
{
    uint64_t last_written = G_DATA_WRITTEN.load();
    uint64_t last_read = G_DATA_READ.load();
    uint64_t last_iocnt = G_IOCOUNT.load();
    while (!Collector_endflag)
    {
        sleep(1);
        uint64_t now_written = G_DATA_WRITTEN.load();
        uint64_t now_read = G_DATA_READ.load();
        uint64_t now_iocnt = G_IOCOUNT.load();

        citem->WBand_persec.emplace_back(now_written - last_written);
        citem->RBand_persec.emplace_back(now_read - last_read);
        citem->Band_persec.emplace_back(now_written + now_read - last_written - last_read);
        citem->IOPS_persec.emplace_back(now_iocnt - last_iocnt);

        last_written = now_written;
        last_read = now_read;
        last_iocnt = now_iocnt;

        // cout << now_written << " " << now_read << " " << now_iocnt << endl;
    }

    vector<uint64_t> *temp = merge_IOLat(G_NUM_WORKERS);//gql-将多个线程的延迟数据合并成一个排序后的vector
    citem->all_IOLat_list.insert(citem->all_IOLat_list.end(), temp->begin(), temp->end());
    vector<uint64_t> *rtemp = merge_ReadIOLat(G_NUM_WORKERS);//gql-将多个线程的读请求延迟数据合并成一个排序后的vector
    citem->read_IOLat_list.insert(citem->read_IOLat_list.end(), rtemp->begin(), rtemp->end());

    citem->all_data_written = G_DATA_WRITTEN.load();
    citem->all_data_read = G_DATA_READ.load();
    citem->all_iocount = G_IOCOUNT.load();

    return;
}

int main(int argc, char *argv[])
{
    string trace_file;
    if (argc > 1)
    {
        trace_file = argv[1];
    }

    ofstream outfile;
    outfile.open("./results/ST_trace_results.txt", ios::out | ios::app);
    if (!outfile.is_open())
    {
        cout << "outfile open error" << endl;
        exit(-1);
    }

    // Trace Files
    printf("Open Trace Files\n");
    string tfile = "./Traces/fileserver_1.log";/*gql-change-log access*/
    // string tfile = "./Traces/mytest.log";/*Gtodo:gql-change-log access*/
    vector<string> v_tfileset{tfile};
    vector<ifstream *> v_tracefile;
    for (size_t i = 0; i < v_tfileset.size(); i++)//gql-v_tfileset中含有多个元素的时候-> 处理多文件的读写
    {
        ifstream *file = new ifstream();
        file->open(v_tfileset[i].c_str(), ios::in);
        if (!file->is_open())
        {
            cout << "file open error" << endl;
            exit(-1);
        }
        v_tracefile.emplace_back(file);
    }

    cout << "Init RAID System" << endl;
    cout << dec << "Number of SSDs: " << NUM_DEV << " | Data Chunks: " << DATACHUNK_NUM << " | Parity Chunks: " << PARITYCHUNK_NUM << endl;
    cout << dec << "Number of Workers: " << G_NUM_WORKERS << endl;
    assert(NUM_DEV == (DATACHUNK_NUM + PARITYCHUNK_NUM));
    srand(time(0));

    //to change 
    string lfile0 = "/dev/nvme0n1";
    string lfile1 = "/dev/nvme1n1";
    string lfile2 = "/dev/nvme2n1";
    string lfile3 = "/dev/nvme3n1";
    string lfile4 = "/dev/nvme4n1";
    // string lfile5 = "/dev/nvme5n1p4";

    // string lfile0 = "/dev/ram0";
    // string lfile1 = "/dev/ram1";
    // string lfile2 = "/dev/ram2";
    // string lfile3 = "/dev/ram3";
    // string lfile4 = "/dev/ram4";
    // string lfile5 = "/dev/ram5";
    //vector<string> v_fileset{lfile0, lfile1, lfile2, lfile3, lfile4, lfile5};
    
    vector<string> v_fileset{lfile0, lfile1, lfile2, lfile3, lfile4};
    assert(NUM_DEV <= v_fileset.size());

    cout << "Open Files" << endl;
    vector<int> v_logfd;
    for (size_t i = 0; i < v_fileset.size(); i++)
    {
        int fd = open(v_fileset[i].c_str(), O_RDWR | O_DIRECT | O_TRUNC);
        assert(fd != -1);
        v_logfd.emplace_back(fd);
    }

    cout << "Generating DevFile" << endl;
    V_DevFiles v_stdFiles;
    for (size_t i = 0; i < (NUM_DEVFILES); i++)
    {
        DevFile *devfile = new DevFile(i, v_fileset[i], v_logfd[i], 0, STRA_SPACE_LEN);
        v_stdFiles.emplace_back(devfile);
    }

    uint64_t user_start_offset = 0;
    uint64_t user_end_offset = USER_SPACE_LEN;
    cout << "Generating MetaMod" << endl;
    MetaMod metamod(user_start_offset, user_end_offset, &v_stdFiles);
    cout << "Generating StorageMod" << endl;
    StorageMod storagemod(&v_stdFiles, &metamod);
    GloStor = &storagemod;

    for (size_t traces = 0; traces < v_tracefile.size(); traces++)//依次遍历v_tfileset文件列表，进行请求处理
    {
        All_Write_Data.store(0);
        All_Read_Data.store(0);
        Cache_Hit.store(0);
        Cache_Miss.store(0);
        RECONST_REQ_NUM.store(0);
        TOTAL_READ_NUM.store(0);

        TTest_Item this_citem;
        uint64_t asize = SSTRIPE_DATASIZE;//gql-条带中数据域的大小

        printf("Run Trace %s\n", v_tfileset.at(traces).c_str());
        vector<bool> trace_iodir;
        vector<uint64_t> trace_off;
        vector<uint64_t> trace_len;//gql-三个变量代表了用于记录IO操作的方向、偏移量和长度

        string line;
        uint64_t lineCount = 0;
        while (getline(*v_tracefile.at(traces), line))
        {
            if (line.empty())
                continue;

            lineCount++;
            vector<string> lineSplit;
            str_split(line, lineSplit, "\t");
            uint64_t offset = atoll(lineSplit[1].c_str());
            uint64_t length = atoll(lineSplit[2].c_str());
            ol_align(length, offset, BLK_SIZE);//gql-将length和offset都对齐到4kb的整数倍
            if (lineSplit[0] == "W" && length > asize)//gql-写的话如果长度>一个条带，就保证条带对齐写，偏移量+长度都对齐到条带大小的整数倍
            {
                ol_align(length, offset, asize);
            }

            if (length > 100 * MB)
            {
                continue;
            }

            trace_iodir.emplace_back(lineSplit[0] == "R" ? 0 : 1);
            trace_off.emplace_back(offset);
            trace_len.emplace_back(length);
        }
        cout << "[Reading] lineCount = " << lineCount << endl;

        printf("Testing Trace\n");
        tic(0);
        thread worker_tid[G_NUM_WORKERS];
        for (size_t th = 0; th < G_NUM_WORKERS; th++)
        {
            vector<bool>::const_iterator iodir_itr = trace_iodir.begin() + th * (trace_iodir.size() / G_NUM_WORKERS);
            vector<uint64_t>::const_iterator off_itr = trace_off.begin() + th * (trace_off.size() / G_NUM_WORKERS);
            vector<uint64_t>::const_iterator len_itr = trace_len.begin() + th * (trace_len.size() / G_NUM_WORKERS);
            worker_tid[th] = thread(worker_run,
                                    th,
                                    &storagemod,
                                    &trace_iodir,
                                    &trace_off,
                                    &trace_len,
                                    // iodir_itr,//不用iodir_itr的原因？？？
                                    // off_itr,
                                    // len_itr,
                                    (trace_iodir.size() / G_NUM_WORKERS));
        }

        thread collector_tid;
        collector_tid = thread(collector_run, &this_citem);
        for (size_t th = 0; th < G_NUM_WORKERS; th++)
        {
            worker_tid[th].join();
        }
        Collector_endflag = true;
        collector_tid.join();

        double timer = toc(0);

        //Gtodo:以下代码为打印输出
        print_throughtput(this_citem.all_data_read+this_citem.all_data_written, trace_off.size(), timer, v_tfileset.at(traces).c_str());
        uint64_t rec_read = RECONST_REQ_NUM.load();
        uint64_t tt_read = TOTAL_READ_NUM.load();
        my_print(v_tfileset.at(traces).c_str(), rec_read, tt_read);

        printf("Printing Results\n");
        outfile << v_tfileset.at(traces) << endl;

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
        for (size_t i = 0; i < 5000; i++)
        {
            float percent = (float)(i * 0.001);
            uint64_t pos = percent * this_citem.all_IOLat_list.size();
            outfile << this_citem.all_IOLat_list.at(pos) << "\t";
        }
        outfile << endl;

        outfile << "Read Latancy CDF: " << endl;//gql-add
        for (size_t i = 0; i < 5000; i++)
        {
            float percent = (float)(i * 0.0002);
            uint64_t pos = percent * this_citem.read_IOLat_list.size();
            outfile << this_citem.read_IOLat_list.at(pos) << "\t";
        }
        outfile << endl;
        outfile << endl;

        G_DATA_WRITTEN.store(0);
        G_DATA_READ.store(0);
        G_IOCOUNT.store(0);
        for (size_t i = 0; i < G_NUM_WORKERS; i++)
        {
            IO_LatArray[i].clear();
            ReadIO_LatArray[i].clear();//gql-add
        }
        Collector_endflag = false;

        iodir_cnt.store(0);
        off_cnt.store(0);
        len_cnt.store(0);
    }

    return 0;
}