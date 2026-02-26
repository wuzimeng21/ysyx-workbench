#include <common.h>

// AM_DEVREG( 5, TIMER_RTC,    RD, int year, month, day, hour, minute, second);
// AM_DEVREG( 6, TIMER_UPTIME, RD, uint64_t us);
int gettimeofday(struct timeval *tv, struct timezone *tz) {
    suseconds_t us = io_read(AM_TIMER_UPTIME).us;
    time_t sec = io_read(AM_TIME_RTC).second;
    if(tv == NULL) {
        assert(tv == NULL);
        return -1;
    }
    tv->tv_sec = sec;
    tv->tv_usec = us;
    return 0;
}