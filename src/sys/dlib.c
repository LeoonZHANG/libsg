/*
 * dlib.c
 * Author: wangwei.
 * Dynamic library handle.
 */

#include <sg/sys/os.h>
#if defined(OS_LNX) || defined(OS_OSX)
# include <dlfcn.h>
#endif
#if defined(OS_WIN)
# include <dlfcn.h>
#endif
#include <string.h>
#include <sg/util/log.h>
#include <sg/sys/dlib.h>
#include <sg/util/assert.h>

const char *sg_dlib_error(void);

sg_dlib *sg_dlib_open(const char *path)
{
    void *handle;

    assert(path);
    assert(strlen(path) > 0);

#if defined(OS_LNX) || defined(OS_OSX)
    handle = dlopen(path, RTLD_LAZY);
#elif defined(OS_WIN)
    handle = (void *)LoadLibrary(path); /* returns HMODULE */
#endif
    if (!handle) {
        sg_log_err("dlopen %s.", sg_dlib_error());
        return NULL;
    }

    return handle;
}

void *sg_dlib_symbol(sg_dlib *handle, const char *symbol)
{
    void *func_addr;

    assert(handle);
    assert(symbol);
    assert(strlen(symbol) > 0);

#if defined(OS_LNX) || defined(OS_OSX)
    func_addr = dlsym(handle, symbol);
#elif defined(OS_WIN)
    func_addr = (void *)GetProcAddress(handle, symbol); /* returns FARPROC */
#endif

    if (!func_addr) {
        sg_log_err("dlib_symbol error, %s.", sg_dlib_error());
        return NULL;
    }

    return func_addr;
}

void sg_dlib_close(sg_dlib **handle)
{
    assert(handle);

    if (!*handle)
        return;

#if defined(OS_LNX) || defined(OS_OSX)
    dlclose(*handle);
#elif defined(OS_WIN)
    FreeLibrary(*handle);
#endif

    *handle = NULL;
}

/* Get last error message. */
const char *sg_dlib_error(void)
{
#if defined(OS_LNX) || defined(OS_OSX)
    return dlerror();
#elif defined(OS_WIN)
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