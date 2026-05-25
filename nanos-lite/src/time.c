#include <common.h>

typedef long suseconds_t;
typedef long time_t;

struct timeval {
  time_t tv_sec;
  suseconds_t tv_usec;
};

struct timezone {
  int tz_minuteswest;
  int tz_dsttime;
};

int gettimeofday(struct timeval *tv, struct timezone *tz) {
  suseconds_t us = io_read(AM_TIMER_UPTIME).us;
  time_t sec = io_read(AM_TIMER_RTC).second;
  if (tv == NULL) {
    return -1;
  }
  tv->tv_sec = sec;
  tv->tv_usec = us;
  return 0;
}
