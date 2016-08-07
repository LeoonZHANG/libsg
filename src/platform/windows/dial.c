/*
 * dial.c
 * Author: wangwei.
 * Windows dial-up connection management.
 */

#include <windows.h>
#include <string.h>
#include <ras.h>
#include <raserror.h>
#include "../../../include/platform/windows/dial.h"

static bool find_conn_desc(LPCWSTR name, LPRASCONNW conn)
{
    LPRASCONNW conns = NULL;
    DWORD buffer_size = 0, n = 0, ret, i;
    BOOL found = FALSE;

    ret = RasEnumconnsW(NULL, &buffer_size, &n);
    if (ret != ERROR_BUFFER_TOO_SMALL)
        goto cleanup;

    conns = (RASCONNW *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, buffer_size);
    conns[0].dwSize = sizeof(RASCONNW);
    ret = RasEnumconnsW(conns, &buffer_size, &n);
    if (ret != ERROR_SUCCESS)
        goto cleanup;

    for (i = 0; i < n; ++i)
        if (wcscmp(name, conns[i].szEntryName) == 0) {
            found = TRUE;
            if (conn)
                *conn = conns[i];
            break;
        }

cleanup:
    if (conns)
        HeapFree(GetProcessHeap(), 0, conns);
    return found;
}

bool dial_find_conn(const char *name)
{
    return find_conn_desc((LPCWSTR)name, NULL);
}

bool dial_open_conn(const char *name)
{
    BOOL pwd;
    DWORD ret, i = 0;
    HRASCONN conn = NULL;
    RASDIALPARAMSW params;

    if (dial_find_conn(name))
        return TRUE;

    memset(&params, 0, sizeof(params));
    params.dwSize = sizeof(params);
    while (i < (sizeof(params.szEntryName) - 1)) {
        if (!(params.szEntryName[i] = name[i]))
            break;
        ++i;
    }
    ret = RasGetEntryDialParamsW(NULL, &params, &pwd);
    if (ERROR_SUCCESS != ret)
        return FALSE;

    ret = RasDialW(NULL, NULL, &params, 0, NULL, &conn);
    if (ERROR_SUCCESS != ret)
        return FALSE;
    return TRUE;
}

bool dial_close_conn(const char *name)
{
    RASCONNW conn;
    RASCONNSTATUSW status;
    DWORD ret;

    if (!find_conn_desc(name, &conn))
        return FALSE;

    status.dwSize = sizeof(status);
    if (RasHangUpW(conn) == ERROR_SUCCESS) {
        do {
            ret = RasGetConnectStatusW(conn, &status);
            Sleep(100);
        } while (ret != ERROR_INVALID_HANDLE);
    }

    return TRUE;
}
