/**
 * cpu.c
 * Get CPU information.
 */

#include <sg/sg.h>
#include <sg/hdw/cpu.h>

#if defined(SG_OS_WINDOWS)
# include <windows.h>
#elif defined(SG_OS_LINUX)
# include <sys/sysinfo.h> /* get_nprocs */
#elif defined(SG_OS_MACOS)
# include <sys/sysctl.h> /* sysctl */
#endif

int sg_cpu_usable_count(void)
{
#if defined(SG_OS_WINDOWS)
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors;
#elif defined(SG_OS_LINUX)
    return get_nprocs();
#elif defined(SG_OS_MACOS)
    int mib[4];
    int cpu_num;
    std::size_t len = sizeof(cpu_num); 
    
    /* set the mib for hw.ncpu */
    mib[0] = CTL_HW;
    mib[1] = HW_AVAILCPU;  /* alternatively, try HW_NCPU */
    
    /* get the number of CPUs from the system */
    sysctl(mib, 2, &cpu_num, &len, NULL, 0);
    
    if (cpu_num < 1) {
        mib[1] = HW_NCPU;
        sysctl(mib, 2, &cpu_num, &len, NULL, 0);
        if (cpu_num < 1)
            cpu_num = 1;
    }

    return cpu_num;
#endif
}