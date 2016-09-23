/*
 * sleep.c
 * Author: wangwei.
 * Unified interface to sleep in milliseconds.
 */

#include <sg/sg.h>

#if defined(SG_OS_WINDOWS)
# include <windows.h>
#else
# include <time.h>
# include <sys/select.h>
#endif

#include <sg/util/log.h>
#include <sg/sys/sleep.h>

#if defined(SG_OS_WINDOWS)
void sg_sleep(uint32_t ms)
{
    Sleep((DWORD)ms);
}
#else
void sg_sleep(uint32_t ms)
{
    int res;
    struct timespec ts;

    ts.tv_sec = ms / 1000;
    ts.tv_nsec = ms % 1000 * 1000000;
    res = nanosleep(&ts, NULL);

    if (res != 0)
        sg_log_err("tv_sec:%ld, tv_nsec:%ld, res:%d.", ts.tv_sec, ts.tv_nsec, res);
}

static void sg_sleep2(int ms)
{
    struct timeval delay;

    delay.tv_sec = 0;
    delay.tv_usec = ms * 1000;

    select(0, NULL, NULL, NULL, &delay);
}
#endif
