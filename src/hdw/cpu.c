/**
 * cpu.c
 * Get CPU information.
 */

#include <sg/sg.h>
#include <sg/hdw/cpu.h>
#if defined(SG_OS_WINDOWS)
# include <windows.h>
#else
# include <sys/sysinfo.h> /* get_nprocs */
#endif

int sg_cpu_usable_count(void)
{
#if defined(SG_OS_WINDOWS)
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors;
#else
    return get_nprocs();
#endif
}