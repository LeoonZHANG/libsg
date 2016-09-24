/**
 * fs.h
 * File system common operations package, including file and directory.
 */

#ifndef LIBSG_FS_H
#define LIBSG_FS_H

#include <sg/sg.h>
#include <sg/str/vsstr.h>
#include <sg/container/vsbuf.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Event for directory seek callback. */
enum sg_fs_dir_seek_event {
    SGFSDIRSEEKEVENT_FOUND = 0, /* new file or directory found */
    SGFSDIRSEEKEVENT_OVER  = 1  /* seeking is done */
};

/* Directory seek callback function type definition.
   data is a string, it ends in terminator '\0'. */
typedef void (*sg_fs_dir_seek_cb_t)(enum sg_fs_dir_seek_event evt,
        const char *path, struct stat *s, void *context);


/****************************************************
 * path
 ****************************************************/

/* Get current working directory which terminates in '/'. */
bool sg_fs_cur_dir(sg_vsstr_t *dir_out);

/* Checks whether a file or directory exists. */
bool sg_fs_path_exists(const char *path);

/* unfinished */
bool sg_fs_full_path_parse(const char *full_path,
        sg_vsstr_t *working_dir, sg_vsstr_t *short_name_no_ext,
        sg_vsstr_t *short_name_with_ext, sg_vsstr_t *ext);


/****************************************************
 * regular file
 ****************************************************/

/* Checks whether a regular file exists. */
bool sg_fs_file_exists(const char *path);

/* Get size of a regular file. */
long sg_fs_file_size(const char *path);

/* Remove regular file. */
bool sg_fs_file_remove(const char *path);

/* Read file to string. */
/* Untested. */
bool sg_fs_file_to_str(const char *path, sg_vsstr_t *out_str);

/* Read file to memory buffer. */
bool sg_fs_file_to_buf(const char *path, sg_vsbuf_t *out_buf);

/* Append data to file. */
bool sg_fs_file_append(const char *path, uint8_t *data, size_t size);

/* Overwrite data to file. */
bool sg_fs_file_overwrite(const char *path, uint8_t *data, size_t size);

#define SG_FS_FREAD_BYTES(buf, size, fp) fread(buf, 1, size, fp);


/****************************************************
 * directory
 ****************************************************/

/* Checks whether a directory exists. */
bool sg_fs_dir_exists(const char *path);

/* Sync seek files or directories.
   If depth is zero. */
bool sg_fs_dir_seek(const char *path, uint8_t depth, sg_fs_dir_seek_cb_t cb, void *context);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_FS_H */
