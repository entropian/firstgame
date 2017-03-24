#pragma once
#include <cstdint>

#if __GNUG__
#include <time.h>
#endif

typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

typedef int8_t I8;
typedef int16_t I16;
typedef int32_t I32;
typedef int64_t I64;

typedef float F32;
typedef double F64;

/* assembly code to read the TSC */
static inline uint64_t RDTSC()
{
  unsigned int hi, lo;
  __asm__ volatile("rdtsc" : "=a" (lo), "=d" (hi));
  return ((uint64_t)hi << 32) | lo;
}

const int NANO_SECONDS_IN_SEC = 1000000000;
struct timespec *TimeSpecDiff(struct timespec *ts1, struct timespec *ts2)
{
  static struct timespec ts;
  ts.tv_sec = ts1->tv_sec - ts2->tv_sec;
  ts.tv_nsec = ts1->tv_nsec - ts2->tv_nsec;
  if (ts.tv_nsec < 0) {
    ts.tv_sec--;
    ts.tv_nsec += NANO_SECONDS_IN_SEC;
  }
  return &ts;
}
 
double g_TicksPerNanoSec;
static void CalibrateTicks()
{
  struct timespec begints, endts;
  uint64_t begin = 0, end = 0;
  clock_gettime(CLOCK_MONOTONIC, &begints);
  begin = RDTSC();
  uint64_t i;
  for (i = 0; i < 1000000; i++); /* must be CPU intensive */
  end = RDTSC();
  clock_gettime(CLOCK_MONOTONIC, &endts);
  struct timespec *tmpts = TimeSpecDiff(&endts, &begints);
  uint64_t nsecElapsed = tmpts->tv_sec * 1000000000LL + tmpts->tv_nsec;
  g_TicksPerNanoSec = (double)(end - begin)/(double)nsecElapsed;
}
 
/* Call once before using RDTSC, has side effect of binding process to CPU1 */
void InitRdtsc()
{
  unsigned long cpuMask;
  cpuMask = 2; // bind to cpu 1
  sched_setaffinity(0, sizeof(cpuMask), (const cpu_set_t*)(&cpuMask));
  CalibrateTicks();
}

float FRAME_TIME = 1.0f / 30.0f;

class Clock
{    
public:
    // Call this when the game first starts up.
    static void init()
    {
        InitRdtsc();
        s_cycles_per_second = (F32)(g_TicksPerNanoSec * NANO_SECONDS_IN_SEC);
    }

    // Construct a clock. (Notice the use of 'explicit' to prevent automatic conversion
    // from F32 to Clock.)
    explicit Clock(F32 start_time_seconds = 0.0f):
    m_time_cycles(secondsToCycles(start_time_seconds)),
        m_time_scale(1.0f), // default to unscaled
        m_is_paused(false) // default to running
    {
    }

    // Return the current time in cycles. NOTE that ew do not return absolute time measurement in
    // floating-point seconds, because a 32-bit float doesn't have enough precision. See calcDeltaSeconds().
    U64 getTimeCycles() const
    {
        return m_time_cycles;
    }

    // Determine the difference between this clock's absolute time and that of another clock, in seconds.
    // We only return time deltas as floating-point seconds, due to the precision limitations of
    // a 32-bit float.
    F32 calcDeltaSeconds(const Clock& other)
    {
        U64 dt = m_time_cycles - other.m_time_cycles;
        return cyclesToSeconds(dt);
    }

    // This function should be called once per frame, with the real measured frame time delta in seconds.
    void update(F32 dt_real_seconds)
    {
        if(!m_is_paused)
        {
            U64 dt_scaled_cycles = secondsToCycles(dt_real_seconds * m_time_scale);
            m_time_cycles += dt_scaled_cycles;
        }               
    }

    void setPaused(bool want_paused)
    {
        m_is_paused = want_paused;
    }

    bool isPaused() const
    {
        return m_is_paused;
    }

    void setTimeScale(F32 scale)
    {
        m_time_scale = scale;
    }

    F32 getTimeScale() const
    {
        return m_time_scale;
    }

    void singleStep()
    {
        if(m_is_paused)
        {
            // Add one ideal frame interval; don't forget to scale it by our current time scale!
            U64 dt_scaled_cycles = secondsToCycles(FRAME_TIME * m_time_scale);
            m_time_cycles + dt_scaled_cycles;
        }
    }
        
protected:
    static inline U64 secondsToCycles(F32 time_seconds)
    {
        return (U64)(time_seconds * s_cycles_per_second);
    }

    // WARNING: Dangerous -- only use to convert small durations into seconds.
    static inline F32 cyclesToSeconds(U64 time_cycles)
    {
        return (F32)time_cycles / s_cycles_per_second;
    }
    
    U64 m_time_cycles;
    F32 m_time_scale;
    bool m_is_paused;
    
    static F32 s_cycles_per_second;
};
