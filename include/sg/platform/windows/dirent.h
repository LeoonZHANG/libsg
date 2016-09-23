/**
 * dirent.h
 * Posix style directory handler for windows only.
 */

#ifndef LIBSG_DIRENT_H
#define LIBSG_DIRENT_H

#include "../../sys/os.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined (SG_OS_WINDOWS)

#include <sys/types.h>
#include <windows.h>

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

/* Directory description. */
typedef struct _dirdesc {
    int     dd_fd;      /* File descriptor associated with directory. */
    long    dd_loc;     /* Offset in current buffer. */
    long    dd_size;    /* Amount of data returned by getdirentries. */
    char    *dd_buf;    /* Data buffer. */
    int     dd_len;     /* Size of data buffer. */
    long    dd_seek;    /* Magic cookie returned by getdirentries. */
    HANDLE  h_find;     /* Find handle. */
} DIR;

struct dirent {
    long d_ino;              /* Inode number, it's zero all the time. */
    off_t d_off;             /* Offset to this dirent. */
    unsigned short d_reclen; /* Structure size. */
    unsigned char d_type;    /* File type. */
    char d_name[MAX_PATH];   /* File name. */
};

/* Open directory stream using plain c string. */
DIR *opendir (const char *path);

/* Read next directory entry. */
struct dirent *readdir (DIR *dir);

/* Close directory stream. */
int closedir (DIR *dir);

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_DIRENT_H */
