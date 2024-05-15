#ifndef DECODE_CC
#define DECODE_CC

#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include <memory.h>
#include "metadata.h"
#include "decode.h"

#include "isa-l.h"

#define RECONST_MOD
// #define DEGRADE
extern atomic_bool proc_end_flag;
extern atomic_uint64_t All_Read_Data;
extern atomic_uint64_t RECONST_REQ_NUM;
extern atomic_uint64_t TOTAL_READ_NUM;
uint64_t SDecodeMod::s_norRead(int thread_id, vector<DIO_Info> v_dios)
{
#ifndef DEGRADE
    if (v_dios.size() == 0)
    {
        return 0;
    }
    uint64_t total_read_size = 0;
    for (size_t ios = 0; ios < v_dios.size(); ios++)
    {
        DIO_Info dio = v_dios.at(ios);
        //将dio的req_id赋值为read
#ifndef RECONST_MOD
        uint64_t usrdata=NVME_RECON_SIG;
#else
        uint64_t usrdata=u64_merg(dio.req_id, NVME_FFAIL_SIG);//gql-userdata-gen
#endif
        DevFile *destdev = v_stdfiles->at(dio.dev_id);
        int fd = destdev->file_fd;
        // printf("Read STD | threadID:%d, deviceID:%d, devoff:%ldkb, len:%ldkb\n", thread_id, dio.dev_id, dio.dev_offset/1024, dio.length/1024);
        iouring_rprep(&stdring[thread_id], fd, dio.buf, dio.dev_offset, dio.length,usrdata);
        total_read_size += dio.length;
        TOTAL_READ_NUM.fetch_add(1);
    }
    uint64_t ret = io_uring_submit(&stdring[thread_id]);
#ifndef RECONST_MOD
    iouring_wait(&stdring[thread_id], ret);
#else
    V_uint *retcqes = new V_uint();
    iouring_rwait(&stdring[thread_id], ret, retcqes);
    //循环遍历retcqe，将失败的读请求重新放入dio中

    for (size_t i = 0; i < retcqes->size(); i++)
    {
        // printf("failed read req:high 32:%lu,low32:%lu\n",sget_high32(retcqes->at(i)),sget_low32(retcqes->at(i)) );
        uint64_t fail_id = sget_high32(retcqes->at(i)) ;
        //tofix
        int vid  = get_id(&v_dios, fail_id);
        //Gtodo: 从失败的读请求中减去ALL_read_data
        if (vid == -1)
        {
            printf("failed to find vid\n");
            continue;
        }
        All_Read_Data.fetch_sub(v_dios.at(vid).length);
        // RECONST_REQ_NUM.fetch_add(1);
        reconstruct_dio(thread_id,v_dios.at(vid));
    }
#endif  

#else

    if (v_dios->size() == 0)
    {
        return 0;
    }

    uint64_t total_read_size = 0;
    for (size_t ios = 0; ios < v_dios->size(); ios++)
    {
        DIO_Info *dio = v_dios->at(ios);

        if (dio->dev_id == 0)
        {
            vector<char *> degrade_buf;
            for (size_t i = 0; i < DATACHUNK_NUM + PARITYCHUNK_NUM; i++)
            {
                DevFile *destdev = v_stdfiles->at(i);
                int fd = destdev->file_fd;

                char *ptr = NULL;
                int ret = posix_memalign((void **)&ptr, ALIGN_SIZE, dio->length);
                assert(ret == 0);
                degrade_buf.emplace_back(ptr);

                iouring_rprep(&stdring[thread_id], fd, ptr, dio->dev_offset, dio->length);
            }
            uint64_t ret = io_uring_submit(&stdring[thread_id]);
            iouring_wait(&stdring[thread_id], ret);

            for (size_t i = 0; i < DATACHUNK_NUM + 1; i++)
            {
                // cout << thread_id << endl;
                ecEncoder *ecdecoder = v_ecDecoder[thread_id];
                uint64_t encode_len = dio->length;
                vector<int> errs_list;
                errs_list.emplace_back(0);
                vector<char *> recovered;
                // ecdecoder->do_decode(degrade_buf, errs_list, &recovered);
                usleep(1);
            }
        }
        else
        {
            DevFile *destdev = v_stdfiles->at(dio->dev_id);
            int fd = destdev->file_fd;
            // printf("Read STD | threadID:%d, fd:%d, devoff:%ld, len:%ld\n", thread_id, fd, dio->dev_offset, dio->length);
            iouring_rprep(&stdring[thread_id], fd, dio->buf, dio->dev_offset, dio->length);
            total_read_size += dio->length;
        }
    }
    uint64_t ret = io_uring_submit(&stdring[thread_id]);
    iouring_wait(&stdring[thread_id], ret);

#endif

    return total_read_size;
}

int SDecodeMod::get_id(vector<DIO_Info> * v_dios, uint64_t reqid)
{
    //循环遍历v_dios，找到req_id对应的dio，返回其在v_dios中的下标
    for (size_t i = 0; i < v_dios->size(); i++)
    {
        if (v_dios->at(i).req_id == reqid)
        {
            return i;
        }
    }
    return -1;
}

bool  SDecodeMod::reconstruct_dio(int thread_id, DIO_Info fail_dio)
{
    if (fail_dio.length > SCHUNK_SIZE)
    {
        printf("reconstruct_dio: fail_dio.length > SCHUNK_SIZE\n");
        return false;
    }

    vector<DIO_Info> recon_dios;
    vector<int> index;
    for (size_t i = 0; i < DATACHUNK_NUM+PARITYCHUNK_NUM; i++)
    {
        index.emplace_back(i);
    }

    int val = fail_dio.dev_id;
    index.erase(remove(index.begin(), index.end(), val), index.end());//从所有设备id中除去因繁忙请求失败对应的设备id
    //使用posix_memalign分配DATACHUNK_NUM个内存，放置到一个容器中
    vector<char *> recondatabuf;
    for (size_t i = 0; i < DATACHUNK_NUM+PARITYCHUNK_NUM-1; i++)
    {
        char *ptr = NULL;
        int ret = posix_memalign((void **)&ptr, ALIGN_SIZE, SCHUNK_SIZE);
        assert(ret == 0);
        recondatabuf.emplace_back(ptr);
    }
    for (size_t i = 0; i < index.size(); i++)
    {
        DIO_Info dio(index[i], recondatabuf.at(i), fail_dio.dev_offset, fail_dio.length);
        recon_dios.emplace_back(dio);
    }

    for (size_t i = 0; i < recon_dios.size(); i++)
    {
        char *readbuf = recon_dios.at(i).buf;
        uint64_t roffset = recon_dios[i].dev_offset;
        uint64_t rlen = recon_dios[i].length;
        DevFile *destdev = v_stdfiles->at(recon_dios[i].dev_id);
        int fd = destdev->file_fd;
        iouring_rprep(&stdring[thread_id], fd, readbuf, roffset, rlen,NVME_RECON_SIG);
    }
    uint64_t rret = io_uring_submit(&stdring[thread_id]);
    iouring_wait(&stdring[thread_id], rret);
    // //to reconstruct data
    // assert (fail_dio.length <= SCHUNK_SIZE);
    encode_dio(recondatabuf, fail_dio.buf, fail_dio.length);
    RECONST_REQ_NUM.fetch_add(1);
    //释放掉分配的DATACHUNK_NUM个内存
    for (size_t i = 0; i < DATACHUNK_NUM+PARITYCHUNK_NUM-1; i++)
    {
        free(recondatabuf.at(i));
    }

    return true;
}

bool SDecodeMod::encode_dio(vector<char *> databufs,char * origin,int dio_len)
{
    //使用isa-l库中的xor_gen函数对databufs中的数据进行异或计算，结果放到origin中
    char *buffs[DATACHUNK_NUM + PARITYCHUNK_NUM];  
    for (size_t i = 0; i < DATACHUNK_NUM; i++)
    {
        buffs[i] = databufs.at(i);
    }
    for (size_t i = 0; i < PARITYCHUNK_NUM; i++)
    {
        buffs[i + DATACHUNK_NUM] = origin;
    }
    if (PARITYCHUNK_NUM == 1)
    {
        xor_gen(DATACHUNK_NUM + PARITYCHUNK_NUM, dio_len, (void **)buffs);
    }
    else if (PARITYCHUNK_NUM == 2)
    {
        pq_gen(DATACHUNK_NUM + PARITYCHUNK_NUM, dio_len, (void **)buffs);
    }
    return true;
}

#endif