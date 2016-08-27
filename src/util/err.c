/*
 * err.c
 * Author: wangwei.
 * Error get and format.
 */

#include <errno.h>
#include <string.h>
#include "../../include/sg/util/err.h"
#include "../../include/sg/str/vlstr.h"
#if defined(OS_WIN)
#include <Windows.h>
#endif

int sg_err_errno_crt(void)
{
    return errno;
}

int sg_err_errno_win(void)
{
    return 0;
}

int sg_err_errno_wsa(void)
{
    return 0;
}

void sg_err_last_reset(void)
{
    errno = 0;
}

int sg_err_last_num(void)
{
    return errno;
}

sg_vlstr_t *err_last_msg(void)
{
    return NULL;
}

sg_vlstr_t *err_num_to_msg(int err_num)
{
    sg_vlstr_t *err_msg;

    #if defined(OS_LNX) || defined(OS_OSX)
    err_msg = sg_vlstralloc2(strerror(errno));
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

    return err_msg;
}
