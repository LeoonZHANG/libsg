/*
 * dirent.c
 * Author: wangwei.
 * Posix style directory handler for windows only.
 */

#include "../include/dirent.h"

#if defined (OS_WIN)

#include <stdio.h>

DIR *opendir(const char *path)
{
    DIR *dir = NULL;
    HANDLE find;
    WIN32_FIND_DATA fd;
    char *seek_path = NULL;

    assert(path);
    assert(strlen(path) > 0);

    seek_path = malloc(strlen(path) + 10);
    if (NULL == seek_path) {
        sg_log_err("Seek path malloc failure.");
        goto end;
    }
    sprintf(seek_path, "%s\\*.*", path);

    find = FindFirstFile(seek_path, &fd);
    if (INVALID_HANDLE_VALUE == find) {
        sg_log_err("FindFirstFile failure %d.", GetLastError());
        goto end;
    }

    dir = (DIR *)malloc(sizeof(DIR));
    if (NULL == dir) {
        sg_log_err("Cannot allocate DIR structure.");
        goto end;
    }

    memset(dir, 0, sizeof(DIR));
    dir->dd_fd = 0; /* Simulate return. */
    dir->h_find = find;

end:
    if (seek_path)
        free(seek_path);
    return dir;
}

struct dirent *readdir(DIR *dir)
{
    int i;
    BOOL res;
    static struct dirent d;
    WIN32_FIND_DATA fd;

    assert(dir);

    res = FindNextFile(dir->h_find, &FileData);
    if (!res)
        return 0;

    snprintf(d.d_name, MAX_PATH, "%s", fd.cFileName);
    d.d_reclen = sizeof(struct dirent);

    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        d.d_type = 2; /* directory */
    else
        d.d_type = 1; /* file */

    return &d;
}

int closedir(DIR *d)
{
    assert(d);

    CloseHandle(d->h_find);
    free(d);

    return 0;
}

#endif
