/*
 * module.c
 * Author: wangwei.
 * APIs about modules.
 */

#include <string.h>
#include <stdio.h>
#include <sg/sys/os.h>
#if defined(OS_LNX)
#include <linux/limits.h>
#elif defined(OS_OSX)
#include <sys/syslimits.h>
#endif
#include <sg/util/assert.h>
#include <sg/util/log.h>

#ifdef OS_LNX
#include <unistd.h> /* getcwd readlink ssize_t */
#endif

#ifdef OS_WIN
#include <direct.h> /* getcwd */
#include <windows.h> /* GetModuleFileName */
#define PATH_MAX MAX_PATH
#endif

#ifdef OS_OSX
#include <mach-o/dyld.h> /* _NSGetExecutablePath */
#endif

#include <sg/sys/module.h>

int sg_cur_dir(char *buf, size_t buf_len)
{
    char *res;

    assert(buf);

#ifdef OS_WIN
    res = _getcwd(buf, (int)buf_len);
#else
    res = getcwd(buf, buf_len);
#endif

    return (res == NULL) ? -1 : 0; /* getcwd return NULL means failure. */
}

#ifdef OS_WIN
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

#ifdef OS_LNX
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

#ifdef OS_OSX
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