/**
 * clock.h
 * Get/set/format/compare and convert date and time.
 */

#ifndef LIBSG_CLOCK_H
#define LIBSG_CLOCK_H

#include <sg/sg.h>

/* Flollowing is a workaround of error: <C2526> for Visual C++, seams a bug of compiler. */
#if defined _MSC_VER && defined __cplusplus
# include <winsock.h>
# include <ctime>
#else
# include <time.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************
 * Get time functions.
 ***************************************/

/* Get unix epoch (00:00:00 1970-01-01 UTC) time in microseconds. */
struct timeval sg_clock_unix_time_us(void);

/* Get unix epoch (00:00:00 1970-01-01 UTC) time in nanoseconds.
   Be influenced by adjtime and NTP. */
struct timespec sg_clock_unix_time_ns(void);

/* Get unix epoch (00:00:00 1970-01-01 UTC) time in seconds. */
time_t sg_clock_unix_time_s(void);

/* Milliseconds from OS boot.
   Be not subject to NTP, but adjtime.
   This clock will pause when system is suspended. */
unsigned long sg_clock_boot_time_ms(void);

/* Thread specific CPU time in milliseconds from OS boot.
   Used for calculate code cost time and performance. */
unsigned long sg_clock_thread_cpu_time_ms(void);

/* Get current time string with given format.
   fmt sample:"%Y-%m-%d %H:%M:%S", "%Y-%m-%d", "%H:%M:%S" */
int sg_clock_curr_date_time(const char *fmt, char *buf, size_t size);


/****************************************
 * Compare time functions.
 ***************************************/

/* Calculate 'later' is how many seconds bigger than 'earlier'. */
time_t sg_clock_unix_time_s_diff(time_t later, time_t earlier);

/* Calculate 'later' is how many seconds bigger than 'earlier'. */
time_t sg_clock_date_time_s_diff(struct tm later, struct tm earlier);


/****************************************
 * Format time functions.
 ***************************************/

/* Convert date time string to datetime.
   fmt sample:"%Y-%m-%d %H:%M:%S" */
int sg_clock_str_to_date_time_s(const char *str, const char *fmt, struct tm *out);

/* Convert datetime to string.
   fmt sample:"%Y-%m-%d %H:%M:%S". */
int sg_clock_date_time_s_to_str(struct tm time, const char *fmt, char *out, size_t out_len);

/* Convert unix time to string. */
int sg_clock_unix_time_ns_to_str(struct timespec time, char *out, size_t out_len);


/****************************************
 * Convert time functions.
 ***************************************/

/* Convert datetime to unix epoch time in seconds. */
time_t sg_clock_date_time_s_to_unix_time_s(struct tm *time);

/* Convert datetime to unix epoch time in nanoseconds, just format convert.
   Returns  0 : succeed.
   Returns -1 : error. */
int sg_clock_date_time_s_to_unix_time_ns(struct tm *time, struct timespec *out);

/*
 * s       seconds      秒
 * ms/msec milliseconds 毫秒
 * us/μs   microseconds 微秒
 * ns      nanoseconds  纳秒
 */

/* Convert nanoseconds to milliseconds. */
#define sg_clock_ns_to_ms(ns) ((ns) / 1000000)

/* Convert microseconds to milliseconds. */
#define sg_clock_us_to_ms(us) ((us) / 1000)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_CLOCK_H */
