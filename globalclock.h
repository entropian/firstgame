#pragma once
#include <time.h>

class GlobalClock
{
public:
    static inline uint64_t RDTSC()
    {
        unsigned int hi, lo;
        __asm__ volatile("rdtsc" : "=a" (lo), "=d" (hi));
        return ((uint64_t)hi << 32) | lo;
    }

    GlobalClock()
        :dt_seconds(0.0f)
    {
        InitRdtsc();
        time_cycles = RDTSC();
    }

    void update()
    {
        uint64_t new_time_cycle = RDTSC();
        dt_seconds = (double)(new_time_cycle - time_cycles) / ticks_per_second;
        time_cycles = new_time_cycle;
    }

    float getDtSeconds()
    {
        return dt_seconds;
    }

    uint64_t getTimeCycles()
    {
        return time_cycles;
    }

private:
    /* Call once before using RDTSC, has side effect of binding process to CPU1 */
    void InitRdtsc()
    {
        unsigned long cpuMask;
        cpuMask = 2; // bind to cpu 1
        sched_setaffinity(0, sizeof(cpuMask), (const cpu_set_t*)(&cpuMask));
        CalibrateTicks();
    }

    static timespec *TimeSpecDiff(timespec *ts1, timespec *ts2)
    {
        static timespec ts;
        ts.tv_sec = ts1->tv_sec - ts2->tv_sec;
        ts.tv_nsec = ts1->tv_nsec - ts2->tv_nsec;
        if (ts.tv_nsec < 0) {
            ts.tv_sec--;
            ts.tv_nsec += NANO_SECONDS_IN_SEC;
        }
        return &ts;
    }

    void CalibrateTicks()
    {
        timespec begints, endts;
        uint64_t begin = 0, end = 0;
        clock_gettime(CLOCK_MONOTONIC, &begints);
        begin = RDTSC();
        uint64_t i;
        for (i = 0; i < 1000000; i++); /* must be CPU intensive */
        end = RDTSC();
        clock_gettime(CLOCK_MONOTONIC, &endts);
        timespec *tmpts = TimeSpecDiff(&endts, &begints);
        uint64_t nsecElapsed = tmpts->tv_sec * 1000000000LL + tmpts->tv_nsec;
        //ticks_per_nanosecond = (double)(end - begin)/(double)nsecElapsed;
        ticks_per_second = (double)(end - begin)/(double)nsecElapsed * NANO_SECONDS_IN_SEC;
    }
    //double ticks_per_nanosecond;
    double ticks_per_second;
    uint64_t time_cycles;
    float dt_seconds;
};
