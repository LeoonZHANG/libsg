/**
 * err.c
 * Error get and format.
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sg/sg.h>
#include <sg/str/vsstr.h>
#include <sg/util/err.h>

#if defined(SG_OS_WINDOWS)
# include <Windows.h>
#endif

#define SG_ERR_LIST_MAX 10000

static char *err_msg_list[SG_ERR_LIST_MAX];

static void sg_err_map(sg_err_t err_no, const char *err_msg)
{
    if (err_no < 0) {
        fprintf(stderr, "err_no %d shouldn\'t be smaller than 0", err_no);
        exit(-1);
    }
    if (err_no > SG_ERR_LIST_MAX) {
        fprintf(stderr, "add too many errno and errmsg");
        exit(-1);
    }
    err_msg_list[err_no] = err_msg;
}

void sg_err_list_init(void)
{
    sg_err_map(SG_OK, "no error");
    sg_err_map(SG_ERR, "normal error");
    sg_err_map(SG_ERR_MALLOC_FAIL, "malloc failed");
    sg_err_map(SG_ERR_NULL_PTR, "null pointer");
}

static void sg_err_add_custom(int err_no, const char *err_msg)
{
    if (err_no <= SG_ERR_MAX) {
        fprintf(stderr, "custom error number shouldn\'t be smaller than %d(SG_ERR_MAX)\n", SG_ERR_MAX);
        exit(-1);
    }
    sg_err_map(err_no, err_msg);
}

const char *sg_err_fmt(sg_err_t err_no)
{
    if (err_no < SG_OK || err_no > SG_ERR_MAX) {
        fprintf(stderr, "errno %d is invalid\n", err_no);
        return NULL;
    }
    return err_msg_list[err_no];
}

/*
sg_vsstr_t *err_num_to_msg(int err_num)
{
    sg_vsstr_t *err_msg;

    #if defined(SG_OS_LINUX) || defined(SG_OS_MACOS)
    err_msg = sg_vsstr_alloc2(strerror(errno));
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

    return err_msg;
}*/