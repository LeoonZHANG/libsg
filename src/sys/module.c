/*
 * module.c
 * Author: wangwei.
 * APIs about modules.
 */

#include <string.h>
#include <stdio.h>
#include <sg/sg.h>
#include <sg/sys/module.h>

#if defined(SG_OS_LINUX)
# include <linux/limits.h>
#elif defined(SG_OS_MACOS)
# include <sys/syslimits.h>
#endif

#ifdef SG_OS_LINUX
# include <unistd.h> /* getcwd readlink ssize_t */
#endif

#ifdef SG_OS_WINDOWS
# include <direct.h> /* getcwd */
# include <windows.h> /* GetModuleFileName */
#endif

#ifdef SG_OS_MACOS
# include <mach-o/dyld.h> /* _NSGetExecutablePath */
#endif

int sg_cur_dir(char *buf, size_t buf_len)
{
    char *res;

    assert(buf);

#ifdef SG_OS_WINDOWS
    res = _getcwd(buf, (int)buf_len);
#else
    res = getcwd(buf, buf_len);
#endif

    return (res == NULL) ? -1 : 0; /* getcwd return NULL means failure. */
}

#ifdef SG_OS_WINDOWS
int sg_module_path(char *buf, size_t buf_len)
{
    DWORD res;

    assert(buf);

    res = GetModuleFileName(NULL /* current process */, (LPSTR)buf, buf_len);

    if (res <= 0)
        sg_log_err("module_dir error:%u.", GetLastError());

    return res > 0 ? 0 : -1;
}
#endif

#ifdef SG_OS_LINUX
int sg_module_path(char *buf, size_t buf_len)
{
    ssize_t size;

    assert(buf);

    size = readlink("/proc/self/exe", buf, buf_len);

    if (size < 0 || size >= buf_len)
        return -1;

    buf[size] = 0;

    return 0;
}
#endif

#ifdef SG_OS_MACOS
int sg_module_path(char *buf, size_t buf_len)
{
    int ret;
    uint32_t size = (uint32_t)buf_len;

    assert(buf);

    ret = _NSGetExecutablePath(buf, &size); /* ret equals zero means succeed. */
    if (ret != 0) {
        sg_log_err("Buffer size %u is smaller than source lenght %u.",
                   buf_len, size);
        return -1;
    }

    return 0;
}
#endif

int sg_module_dir(char *buf, size_t buf_len)
{
    int ret;
    char *end;

    assert(buf);

    ret = sg_module_path(buf, buf_len);
    if (ret != 0)
        return -1;

    end = strrchr(buf, '/');
    if (!end)
        return -1;
    end[1] = 0;

    return 0;
}

int sg_module_name(char *buf, size_t buf_len)
{
    int ret;
    char swap[PATH_MAX];
    char *start;

    assert(buf);

    ret = sg_module_path(swap, PATH_MAX);
    if (ret != 0)
        return -1;

    start = strrchr(swap, '/');
    if (!start)
        return -1;
    snprintf(buf, buf_len, "%s", start + 1);
    return 0;
}