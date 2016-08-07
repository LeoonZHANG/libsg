/*
 * time.c
 * Author: wangwei.
 * Get/set/format/compare and convert time.
 */

#ifndef __USE_XOPEN
# define __USE_XOPEN /* strptime */
#endif
#ifndef __USE_XOPEN2K8
# define __USE_XOPEN2K8 /* strftime */
#endif

#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include "../../include/sys/time.h"
#include "../../include/util/assert.h"


/****************************************
 * Get time functions.
 ***************************************/

struct timeval sg_unix_time_us(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return tv;
}

struct timespec sg_unix_time_ns(void)
{
    struct timespec t;

    /* Be influenced by adjtime and NTP. */
    clock_gettime(CLOCK_REALTIME, &t);

    return t;
}

time_t sg_unix_time_s(void)
{
    time_t res = 0;

    res = time(NULL);
    assert(res > 0);

    return res;
}

unsigned long sg_boot_time_ms(void)
{
    struct timespec t;
    unsigned long ms; /* milliseconds */

    /* Unsigned long (32-bit OS) could describe
       milliseconds from OS boot for 8 years. */
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    ms = (unsigned long)(t.tv_sec * 1000) + (unsigned long)(t.tv_nsec / 1000000);

    return ms;
}

unsigned long sg_thread_cpu_time_ms(void)
{
    struct timespec t;
    unsigned long ms; /* milliseconds */

    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t);
    ms = (unsigned long)(t.tv_sec * 1000)
         + (unsigned long)(t.tv_nsec / 1000000);

    return ms;
}

int sg_curr_date_time(const char *fmt, char *buf, size_t size)
{
    time_t t_t;
    struct tm *t_m;

    t_t = time(NULL);
    t_m = localtime(&t_t);
    return sg_date_time_s_to_str(*t_m, fmt, buf, size);
}


/****************************************
 * Compare time functions.
 ***************************************/

time_t sg_unix_time_s_diff(time_t later, time_t earlier)
{
    return (later - earlier);
}

time_t sg_date_time_s_diff(struct tm later, struct tm earlier)
{
    return (sg_date_time_s_to_unix_time_s(&later)
            - sg_date_time_s_to_unix_time_s(&earlier));
}


/****************************************
 * Format time functions.
 ***************************************/

int sg_str_to_date_time_s(const char *str, const char *fmt, struct tm *out)
{
    assert(str);
    assert(fmt);
    assert(out);

    strptime(str, fmt, out);

    return 0;
}

int sg_date_time_s_to_str(struct tm time, const char *fmt, char *out, size_t out_len)
{
    struct tm swap;

    assert(fmt);
    assert(out);
    assert(out_len > 0);

    swap = time;
    out[0] = 0;
    strftime(out, out_len, fmt, &swap);

    return 0;
}

int sg_unix_time_ns_to_str(struct timespec time, char *out, size_t out_len)
{
    time_t tm;

    tm = time.tv_sec;
    snprintf(out, out_len, "%s", ctime(&tm));

    return 0;
}


/****************************************
 * Convert time functions.
 ***************************************/

time_t sg_date_time_s_to_unix_time_s(struct tm *time)
{
    time_t res;

    assert(time);

    res = mktime(time);
    res = (res < 0) ? 0 : res;
    return res;
}

int sg_date_time_s_to_unix_time_ns(struct tm *time, struct timespec *out)
{
    assert(time);
    assert(out);

    out->tv_sec  = mktime(time);
    out->tv_nsec = 0;
    return 0;
}
