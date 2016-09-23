/*
 * dynlib.c
 * Author: wangwei.
 * Dynamic library handle.
 */

#include <string.h>
#include <sg/sg.h>
#include <sg/sys/dynlib.h>

#if defined(SG_OS_LINUX) || defined(SG_OS_MACOS)
# include <dlfcn.h>
#elif defined(SG_OS_WINDOWS)
# include <Windows.h>
#endif

const char *sg_dynlib_error(void);

sg_dynlib *sg_dynlib_open(const char *path)
{
    void *handle;

    assert(path);
    assert(strlen(path) > 0);

#if defined(SG_OS_LINUX) || defined(SG_OS_MACOS)
    handle = dlopen(path, RTLD_LAZY);
#elif defined(SG_OS_WINDOWS)
    handle = (void *)LoadLibrary(path); /* returns HMODULE */
#endif
    if (!handle) {
        sg_log_err("dlopen %s.", sg_dynlib_error());
        return NULL;
    }

    return handle;
}

void *sg_dynlib_symbol(sg_dynlib *handle, const char *symbol)
{
    void *func_addr;

    assert(handle);
    assert(symbol);
    assert(strlen(symbol) > 0);

#if defined(SG_OS_LINUX) || defined(SG_OS_MACOS)
    func_addr = dlsym(handle, symbol);
#elif defined(SG_OS_WINDOWS)
    func_addr = (void *)GetProcAddress(handle, symbol); /* returns FARPROC */
#endif

    if (!func_addr) {
        sg_log_err("dynlib_symbol error, %s.", sg_dynlib_error());
        return NULL;
    }

    return func_addr;
}

void sg_dynlib_close(sg_dynlib **handle)
{
    assert(handle);

    if (!*handle)
        return;

#if defined(SG_OS_LINUX) || defined(SG_OS_MACOS)
    dlclose(*handle);
#elif defined(SG_OS_WINDOWS)
    FreeLibrary(*handle);
#endif

    *handle = NULL;
}

/* Get last error message. */
const char *sg_dynlib_error(void)
{
#if defined(SG_OS_LINUX) || defined(SG_OS_MACOS)
    return dlerror();
#elif defined(SG_OS_WINDOWS)
    DWORD errno, sys_locale;
    errno = GetLastError();
    HLOCAL handle_local = NULL;
    sys_locale = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
                  | FORMAT_MESSAGE_IGNORE_INSERTS
                  | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                  NULL, errno, sys_locale, (LPWSTR)&handle_local, 0, NULL);
    return LocalLock(handle_local);
#endif
}