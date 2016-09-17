/*
 * fs.h
 * Author: wangwei.
 * Regular file and directory common operations package.
 */

#ifndef LIBSG_FS_H
#define LIBSG_FS_H

#include "../str/bool.h"
#include "../str/vlstr.h"
#include "../container/vlbuf.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Event for directory seek callback. */
enum sg_fs_dir_seek_event {
    SGFSDIRSEEKEVENT_FOUND = 0, /* new file or directory found */
    SGFSDIRSEEKEVENT_OVER  = 2, /* seeking is done */
};

/* Directory seek callback function type definition.
   data is a string, it ends in terminator '\0'. */
typedef void (*sg_fs_dir_seek_callback)(enum sg_fs_dir_seek_event evt,
                                     const char *path, struct stat *s, void *context);

/* Checks whether a file or directory exists. */
int sg_fs_path_exists(const char *path);

/* Checks whether a regular file exists. */
bool sg_fs_reg_file_exists(const char *path);

/* Get size of a regular file. */
long sg_fs_reg_file_size(const char *path);

/* Remove regular file. */
int sg_fs_reg_file_remove(const char *path);

/* Checks whether a directory exists. */
int sg_fs_dir_exists(const char *path);

/* Sync seek files or directories.
   If depth is zero. */
int sg_fs_dir_seek(const char *dir_path, uint8_t depth, sg_dir_seek_callback cb, void *context);

/* Get suffix name of full path with the dot(.),
 * remember to free it. */
int sg_fs_file_get_ext(const char *filename, int uppercase, sg_vlstr_t *ext);

/* Read file to string. */
/* Untested. */
int sg_fs_file_to_str(const char *filename, sg_vlstr_t *str);

/* Read file to memory buffer. */
int sg_fs_file_to_buf(const char *filename, uint8_t **buffer, size_t *size);

/* Append data to file. */
int sg_fs_file_append(const char *filename, uint8_t *data, size_t size);

/* Overwrite data to file. */
int sg_fs_file_overwrite(const char *filename, uint8_t *data, size_t size);

#define SG_FS_FREAD_BYTES(buf, size, fp) fread(buf, 1, size, fp);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_FS_H */
