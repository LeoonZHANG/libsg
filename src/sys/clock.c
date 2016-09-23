/*
 * clock.c
 * Author: wangwei.
 * Get/set/format/compare and convert date and time.
 */

/* these macros must before #include <time.h> */
#ifndef __USE_XOPEN
# define __USE_XOPEN /* strptime */
#endif
#ifndef __USE_XOPEN2K8
# define __USE_XOPEN2K8 /* strftime */
#endif

#include <stdio.h>
#include <time.h>
#include <sg/sg.h>
#include <sg/sys/clock.h>

#ifdef SG_OS_MACOS
# include <mach/clock.h>
# include <mach/mach.h>
#endif

#if defined(SG_OS_WINDOWS)
# include <Windows.h>
#else
# include <sys/time.h>
#endif



#if defined(SG_OS_WINDOWS)
static int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    static const __int64 EPOCH = ((__int64)116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    __int64    time;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    time = ((__int64)file_time.dwLowDateTime);
    time += ((__int64)file_time.dwHighDateTime) << 32;

    tp->tv_sec = (long)((time - EPOCH) / 10000000L);
    tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
    return 0;
}

const char * strp_weekdays[] =
{ "sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday" };
const char * strp_monthnames[] =
{ "january", "february", "march", "april", "may", "june", "july", "august", "september", "october", "november", "december" };
BOOL strp_atoi(const char * s, int * result, int low, int high, int offset)
{
    BOOL worked = FALSE;
    char * end;
    unsigned long num = strtoul(s, &end, 10);
    if (num >= (unsigned long)low && num <= (unsigned long)high)
    {
        *result = (int)(num + offset);
        s = end;
        worked = TRUE;
    }
    return worked;
}
char * strptime(const char *s, const char *format, struct tm *tm)
{
    BOOL working = TRUE;
    while (working && *format && *s)
    {
        switch (*format)
        {
        case '%':
        {
            ++format;
            switch (*format)
            {
            case 'a':
            case 'A': // weekday name
                tm->tm_wday = -1;
                working = FALSE;
                for (int i = 0; i < 7; ++i)
                {
                    size_t len = strlen(strp_weekdays[i]);
                    if (!strnicmp(strp_weekdays[i], s, len))
                    {
                        tm->tm_wday = i;
                        s += len;
                        working = TRUE;
                        break;
                    }
                    else if (!strnicmp(strp_weekdays[i], s, 3))
                    {
                        tm->tm_wday = i;
                        s += 3;
                        working = TRUE;
                        break;
                    }
                }
                break;
            case 'b':
            case 'B':
            case 'h': // month name
                tm->tm_mon = -1;
                working = FALSE;
                for (size_t i = 0; i < 12; ++i)
                {
                    size_t len = strlen(strp_monthnames[i]);
                    if (!strnicmp(strp_monthnames[i], s, len))
                    {
                        tm->tm_mon = i;
                        s += len;
                        working = TRUE;
                        break;
                    }
                    else if (!strnicmp(strp_monthnames[i], s, 3))
                    {
                        tm->tm_mon = i;
                        s += 3;
                        working = TRUE;
                        break;
                    }
                }
                break;
            case 'd':
            case 'e': // day of month number
                working = strp_atoi(s, &tm->tm_mday, 1, 31, 0);
                break;
            case 'D': // %m/%d/%y
            {
                const char * s_save = s;
                working = strp_atoi(s, &tm->tm_mon, 1, 12, -1);
                if (working && *s == '/')
                {
                    ++s;
                    working = strp_atoi(s, &tm->tm_mday, 1, 31, 0);
                    if (working && *s == '/')
                    {
                        ++s;
                        working = strp_atoi(s, &tm->tm_year, 0, 99, 0);
                        if (working && tm->tm_year < 69)
                            tm->tm_year += 100;
                    }
                }
                if (!working)
                    s = s_save;
            }
            break;
            case 'H': // hour
                working = strp_atoi(s, &tm->tm_hour, 0, 23, 0);
                break;
            case 'I': // hour 12-hour clock
                working = strp_atoi(s, &tm->tm_hour, 1, 12, 0);
                break;
            case 'j': // day number of year
                working = strp_atoi(s, &tm->tm_yday, 1, 366, -1);
                break;
            case 'm': // month number
                working = strp_atoi(s, &tm->tm_mon, 1, 12, -1);
                break;
            case 'M': // minute
                working = strp_atoi(s, &tm->tm_min, 0, 59, 0);
                break;
            case 'n': // arbitrary whitespace
            case 't':
                while (isspace((int)*s))
                    ++s;
                break;
            case 'p': // am / pm
                if (!strnicmp(s, "am", 2))
                { // the hour will be 1 -> 12 maps to 12 am, 1 am .. 11 am, 12 noon 12 pm .. 11 pm
                    if (tm->tm_hour == 12) // 12 am == 00 hours
                        tm->tm_hour = 0;
                }
                else if (!strnicmp(s, "pm", 2))
                {
                    if (tm->tm_hour < 12) // 12 pm == 12 hours
                        tm->tm_hour += 12; // 1 pm -> 13 hours, 11 pm -> 23 hours
                }
                else
                    working = FALSE;
                break;
            case 'r': // 12 hour clock %I:%M:%S %p
            {
                const char * s_save = s;
                working = strp_atoi(s, &tm->tm_hour, 1, 12, 0);
                if (working && *s == ':')
                {
                    ++s;
                    working = strp_atoi(s, &tm->tm_min, 0, 59, 0);
                    if (working && *s == ':')
                    {
                        ++s;
                        working = strp_atoi(s, &tm->tm_sec, 0, 60, 0);
                        if (working && isspace((int)*s))
                        {
                            ++s;
                            while (isspace((int)*s))
                                ++s;
                            if (!strnicmp(s, "am", 2))
                            { // the hour will be 1 -> 12 maps to 12 am, 1 am .. 11 am, 12 noon 12 pm .. 11 pm
                                if (tm->tm_hour == 12) // 12 am == 00 hours
                                    tm->tm_hour = 0;
                            }
                            else if (!strnicmp(s, "pm", 2))
                            {
                                if (tm->tm_hour < 12) // 12 pm == 12 hours
                                    tm->tm_hour += 12; // 1 pm -> 13 hours, 11 pm -> 23 hours
                            }
                            else
                                working = FALSE;
                        }
                    }
                }
                if (!working)
                    s = s_save;
            }
            break;
            case 'R': // %H:%M
            {
                const char * s_save = s;
                working = strp_atoi(s, &tm->tm_hour, 0, 23, 0);
                if (working && *s == ':')
                {
                    ++s;
                    working = strp_atoi(s, &tm->tm_min, 0, 59, 0);
                }
                if (!working)
                    s = s_save;
            }
            break;
            case 'S': // seconds
                working = strp_atoi(s, &tm->tm_sec, 0, 60, 0);
                break;
            case 'T': // %H:%M:%S
            {
                const char * s_save = s;
                working = strp_atoi(s, &tm->tm_hour, 0, 23, 0);
                if (working && *s == ':')
                {
                    ++s;
                    working = strp_atoi(s, &tm->tm_min, 0, 59, 0);
                    if (working && *s == ':')
                    {
                        ++s;
                        working = strp_atoi(s, &tm->tm_sec, 0, 60, 0);
                    }
                }
                if (!working)
                    s = s_save;
            }
            break;
            case 'w': // weekday number 0->6 sunday->saturday
                working = strp_atoi(s, &tm->tm_wday, 0, 6, 0);
                break;
            case 'Y': // year
                working = strp_atoi(s, &tm->tm_year, 1900, 65535, -1900);
                break;
            case 'y': // 2-digit year
                working = strp_atoi(s, &tm->tm_year, 0, 99, 0);
                if (working && tm->tm_year < 69)
                    tm->tm_year += 100;
                break;
            case '%': // escaped
                if (*s != '%')
                    working = FALSE;
                ++s;
                break;
            default:
                working = FALSE;
            }
        }
        break;
        case ' ':
        case '\t':
        case '\r':
        case '\n':
        case '\f':
        case '\v':
            // zero or more whitespaces:
            while (isspace((int)*s))
                ++s;
            break;
        default:
            // match character
            if (*s != *format)
                working = FALSE;
            else
                ++s;
            break;
        }
        ++format;
    }
    return (working ? (char *)s : 0);
}
#endif



/****************************************
 * Get time functions.
 ***************************************/

struct timeval sg_clock_unix_time_us(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return tv;
}

struct timespec sg_clock_unix_time_ns(void)
{
    struct timespec t;

#ifdef SG_OS_MACOS
    /* clock_gettime wasn't implemented before macOS 10.12 */
    clock_serv_t cclock;
    mach_timespec_t mts;

    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    t.tv_sec = mts.tv_sec;
    t.tv_nsec = mts.tv_nsec;
    mach_port_deallocate(mach_task_self(), cclock);
#elif defined(SG_OS_WINDOWS)
    /* FIXME: find out a higher resolution solution */
    struct timeval tv;
    gettimeofday(&tv, NULL);
    t.tv_sec = tv.tv_sec;
    t.tv_nsec = tv.tv_usec * 1000;
#else
    /* Be influenced by adjtime and NTP. */
    clock_gettime(CLOCK_REALTIME, &t);
#endif

    return t;
}

time_t sg_clock_unix_time_s(void)
{
    time_t res = 0;

    res = time(NULL);
    assert(res > 0);

    return res;
}

unsigned long sg_clock_boot_time_ms(void)
{
    unsigned long ms; /* milliseconds */

#ifdef SG_OS_MACOS
    /* clock_gettime wasn't implemented before macOS 10.12 */
    clock_serv_t cclock;
    mach_timespec_t mts;

    host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    ms = (unsigned long)(mts.tv_sec * 1000) + (unsigned long)(mts.tv_nsec / 1000000);
#elif defined(SG_OS_WINDOWS)
    struct timeval tv = sg_unix_time_us();
    ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;
#else
    struct timespec t;
    /* Unsigned long (32-bit OS) could describe
       milliseconds from OS boot for 8 years. */
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    ms = (unsigned long)(t.tv_sec * 1000) + (unsigned long)(t.tv_nsec / 1000000);
#endif

    return ms;
}

unsigned long sg_clock_thread_cpu_time_ms(void)
{
    struct timespec t;
    unsigned long ms; /* milliseconds */

#ifdef SG_OS_MACOS
    /* clock_gettime wasn't implemented before macOS 10.12 */
    thread_port_t thread = mach_thread_self();

    mach_msg_type_number_t count = THREAD_BASIC_INFO_COUNT;
    thread_basic_info_data_t info;

    int kr = thread_info(thread, THREAD_BASIC_INFO, (thread_info_t) &info, &count);
    if (kr != KERN_SUCCESS) {
        return 0;
    }

    t.tv_sec = info.user_time.seconds + info.system_time.seconds;
    t.tv_nsec = info.user_time.microseconds * 1000 + info.system_time.microseconds * 1000;

    mach_port_deallocate(mach_task_self(), thread);
#elif defined(SG_OS_WINDOWS)
    FILETIME create, exit, kernel, user;
    GetThreadTimes(GetCurrentThread, &create, &exit, &kernel, &user);
    SYSTEMTIME kernel_time, user_time;
    FileTimeToSystemTime(&kernel, &kernel_time);
    FileTimeToSystemTime(&user, &user_time);
    t.tv_sec = (((kernel_time.wDay * 24 + kernel_time.wHour) * 60 + kernel_time.wMinute) * 60) + kernel_time.wSecond
        + (((user_time.wDay * 24 + user_time.wHour) * 60 + user_time.wMinute) * 60) + user_time.wSecond;
    t.tv_nsec = (kernel_time.wMilliseconds + user_time.wMilliseconds) * 1000000;
#else
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t);
#endif
    ms = (unsigned long)(t.tv_sec * 1000)
         + (unsigned long)(t.tv_nsec / 1000000);

    return ms;
}

int sg_clock_curr_date_time(const char *fmt, char *buf, size_t size)
{
    time_t t_t;
    struct tm *t_m;

    t_t = time(NULL);
    t_m = localtime(&t_t);
    return sg_clock_date_time_s_to_str(*t_m, fmt, buf, size);
}


/****************************************
 * Compare time functions.
 ***************************************/

time_t sg_clock_unix_time_s_diff(time_t later, time_t earlier)
{
    return (later - earlier);
}

time_t sg_clock_date_time_s_diff(struct tm later, struct tm earlier)
{
    return (sg_clock_date_time_s_to_unix_time_s(&later)
            - sg_clock_date_time_s_to_unix_time_s(&earlier));
}


/****************************************
 * Format time functions.
 ***************************************/

int sg_clock_str_to_date_time_s(const char *str, const char *fmt, struct tm *out)
{
    assert(str);
    assert(fmt);
    assert(out);

    strptime(str, fmt, out);

    return 0;
}

int sg_clock_date_time_s_to_str(struct tm time, const char *fmt, char *out, size_t out_len)
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

int sg_clock_unix_time_ns_to_str(struct timespec time, char *out, size_t out_len)
{
    time_t tm;

    tm = time.tv_sec;
    snprintf(out, out_len, "%s", ctime(&tm));

    return 0;
}


/****************************************
 * Convert time functions.
 ***************************************/

time_t sg_clock_date_time_s_to_unix_time_s(struct tm *time)
{
    time_t res;

    assert(time);

    res = mktime(time);
    res = (res < 0) ? 0 : res;
    return res;
}

int sg_clock_date_time_s_to_unix_time_ns(struct tm *time, struct timespec *out)
{
    assert(time);
    assert(out);

    out->tv_sec  = mktime(time);
    out->tv_nsec = 0;
    return 0;
}