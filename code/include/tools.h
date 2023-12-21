#ifndef TOOLS_H
#define TOOLS_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <algorithm>
#include <vector>
#include <atomic>
#include <liburing.h>
#include "define.h"
#include <sys/time.h>

using namespace std;

#ifdef __FreeBSD__
#define CLOCK_ID CLOCK_MONOTONIC_PRECISE
#else
#define CLOCK_ID CLOCK_MONOTONIC
#endif

void dump(unsigned char *buf, int len);
vector<uint64_t> randperm(int Num);
uint64_t get_nowtimeus();
void tic(int i);
double toc(int i);
pair<float, float> print_throughtput(long long data_length, int io_count, double past_time, const char *info);
void my_print(const char *info, long int recs_read,long int total_read);

void ptic(int i);
uint64_t ptoc(int i);

void lat_tic(int i);
uint64_t lat_toc(int i);

vector<uint64_t> *merge_IOLat(int thread_count);
vector<uint64_t> *merge_ReadIOLat(int thread_count);//gql-add
vector<uint64_t> *show_IOLat(int thread_count);
void clear_IOLat();

uint64_t comm_tic(timespec *start_time);
uint64_t comm_toc(timespec *start_time, timespec *end_time);

int DropCaches(int drop); // drop CPU cache
bool isBadPtr(void *p);
uint64_t o_align(uint64_t offset, uint64_t align);
uint64_t l_align(uint64_t length, uint64_t align);
void ol_align(uint64_t &length, uint64_t &offset, uint64_t align);

// io_uring
void iouring_wprep(io_uring *ring, int fd, char *buf, uint64_t dev_off, uint64_t length);
void iouring_rprep(io_uring *ring, int fd, char *buf, uint64_t dev_off, uint64_t length, uint64_t req_flag);
uint64_t iouring_wsubmit(io_uring *ring, int fd, char *buf, uint64_t dev_off, uint64_t length);
uint64_t iouring_rsubmit(io_uring *ring, int fd, char *buf, uint64_t dev_off, uint64_t length);
bool iouring_wait(io_uring *ring, uint wait_count);
bool iouring_rwait(io_uring *ring, uint wait_count,V_uint *retinfo);/*gql-add，wait info of failed read req*/

//for uint64_t calculation
void u64_dev(uint64_t origin, uint64_t &high, uint64_t &low);
uint64_t u64_merg(uint64_t high, uint64_t low);
uint64_t sget_low32(uint64_t data);
uint64_t sget_high32(uint64_t data);



#endif
