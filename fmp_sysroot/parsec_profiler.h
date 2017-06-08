#pragma once

#include <stdint.h>

enum {
    PROFILE_WORK = 0,
    PROFILE_WORK_SERIAL,
    PROFILE_FILE_IO,
    PROFILE_MALLOCATOR,
    PROFILE_SYNC, // pthread create, POSIX mutex, cond, mutex
    PROFILE_NUMBER,
};

#if !defined(DEFINE_PROFILE_DATA_HERE)
#define DEFINE_PROFILE_DATA(prototype, intval) extern prototype;
#else
#define DEFINE_PROFILE_DATA(prototype, intval) prototype=intval;
#endif

DEFINE_PROFILE_DATA(uint64_t _fork_number, 1)
DEFINE_PROFILE_DATA(uint64_t _fork_tid_0, 0)
DEFINE_PROFILE_DATA(uint64_t _fork_threads[128],  {1})
DEFINE_PROFILE_DATA(uint64_t _per_thread_profiles[1024][PROFILE_NUMBER],  {})

#if 0
#if defined(PROFILE_TILEGX_72)
#define PROFILE_CPU_HZ (1000000000)
#else
#define PROFILE_CPU_HZ (1200000000)
#endif

#define PROFILE_DATA_TO_MS(data) ((data)/(PROFILE_CPU_HZ/1000))
#endif
#define PROFILE_DATA_TO_MS(data) ((data)/(1000000))

#define PROFILER_TIMER_NOW           (__insn_mfspr(0x2781/*SPR_CYCLE*/))
#define PROFILER_TIMER(timer)        profiler_timer_##timer
#define PROFILER_TIMER_UPDATE(timer) PROFILER_TIMER(timer)=PROFILER_TIMER_NOW
#define PROFILER_TIMER_INIT(timer)   uint64_t PROFILER_TIMER_UPDATE(timer)

#define PROFILER_FORK_TID(tid)   (_fork_tid_0 + tid)
#define PROFILER_ADD_TIME(p,tid,time2,time1) _per_thread_profiles[tid][p]+=time2-time1

static inline
void PROFILER_NEW_FORK(uint64_t threads) {
    _fork_tid_0 += _fork_threads[_fork_number - 1];
    _fork_threads[_fork_number++] = threads;
}

static inline
void PROFILER_DUMP(uint64_t begin, uint64_t nthreads) {
    uint64_t max_tid = begin, max_ms = 0, tid;
    for (tid = begin; tid < begin + nthreads; tid++) {
        printf("thread %lu:\n", tid);
        uint64_t total = 0;
#define DUMP_AND_SUM(p) printf("" #p ": %lu Mcycles\n", \
        PROFILE_DATA_TO_MS(_per_thread_profiles[tid][p])); \
        total += PROFILE_DATA_TO_MS(_per_thread_profiles[tid][p])
        DUMP_AND_SUM(PROFILE_WORK);
        DUMP_AND_SUM(PROFILE_WORK_SERIAL);
        DUMP_AND_SUM(PROFILE_FILE_IO);
        DUMP_AND_SUM(PROFILE_MALLOCATOR);
        DUMP_AND_SUM(PROFILE_SYNC);
        printf("total: %lu\n", total);
        if (total > max_ms) { max_tid = tid; max_ms = total; }
    }
    printf("MAX is thread %lu, %lu Mcycles in total\n", max_tid, max_ms);
}

static inline
void PROFILER_DUMP_ALL() {
    uint64_t begin = 0, nthreads = 0, i;
    for (i = 0; i < _fork_number; i++) {
        nthreads = _fork_threads[i];
        printf("fork %lu thread %lu to %lu\n", i, begin, begin + nthreads - 1);
        PROFILER_DUMP(begin, nthreads);
        begin += nthreads;
    }
}
