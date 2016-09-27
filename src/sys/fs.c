/**
 * fs.c
 * File system common operations package, including file and directory.
 */

#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sg/sg.h>
#include <sg/sys/fs.h>
#include <sg/str/string.h>

#if defined(SG_OS_WINDOWS)
# include <sg/platform/windows/dirent.h>
# include <direct.h> /* getcwd */
#endif

#if defined(SG_OS_LINUX)
# include <bits/errno.h>
# include <dirent.h>
# include <stdint.h>
# include <unistd.h> /* getcwd readlink ssize_t */
#endif

#if defined(SG_OS_MACOS)
# include <dirent.h>
# include <zconf.h> /* getcwd */
#endif


int dir_seek_by_depth(const char *dir_path, uint8_t cur_depth,
                      uint8_t max_depth, sg_fs_dir_seek_cb_t cb, void *context);


/****************************************************
 * path
 ****************************************************/

bool sg_fs_cur_dir(sg_vsstr_t *dir_out)
{
    char *res;
    char tmp[SG_LIMIT_PATH_MAX] = {0};

    SG_ASSERT(dir_out);

#ifdef SG_OS_WINDOWS
    res = _getcwd(tmp, (int)SG_LIMIT_PATH_MAX);
#else
    res = getcwd(tmp, SG_LIMIT_PATH_MAX);
#endif

    return (res == NULL) ? false : true; /* getcwd return NULL means failure. */
}

bool sg_fs_path_exists(const char *path)
{
    struct stat s;

    SG_ASSERT(path);
    SG_ASSERT(strlen(path) > 0);

    errno = 0;
    stat(path, &s);

    return (errno == ENOENT) ? false : true; /* does not exist */
}

bool sg_fs_full_path_parse(const char *full_path,
                           sg_vsstr_t *working_dir, sg_vsstr_t *short_name_no_ext,
                           sg_vsstr_t *short_name_with_ext, sg_vsstr_t *ext)
{
    sg_vsstr_t *v;
    char *dot;
    char *end;
    char *tmp;

    SG_ASSERT(full_path);

    if (ext) {
        dot = strrchr(sg_vsstr_raw(full_path), '.');
        /* Dot is the last character before terminator. */
        if (dot && dot != sg_vsstr_raw(full_path) + sg_vsstr_len(full_path) - 1)
            sg_vsstr_cpy(ext, dot);
    }

    if (working_dir) {
        sg_vsstr_cpy(working_dir, full_path);

        end = strrchr(sg_vsstr_raw(working_dir), '/');
        if (end)
            end[1] = 0;
    }

    return true;
}



/****************************************************
 * regular file
 ****************************************************/

bool sg_fs_file_exists(const char *path)
{
    struct stat s;

    SG_ASSERT(path);
    SG_ASSERT(strlen(path) > 0);

    errno = 0;
    stat(path, &s);

    if (errno == ENOENT) /* does not exist */
        return false;

    return (S_IFREG & s.st_mode) ? true : false; /* is regular file or not */
}

long sg_fs_file_size(const char *path)
{
    struct stat s;

    SG_ASSERT(path);
    SG_ASSERT(strlen(path) > 0);

    errno = 0;
    stat(path, &s);

    if (errno == ENOENT) /* does not exist */
        return -1;

    if (!(S_IFREG & s.st_mode)) /* is regular file or not */
        return -1;

    return s.st_size;
}

bool sg_fs_file_remove(const char *path)
{
    SG_ASSERT(path);

    return (remove(path) == 0) ? true : false;
}

bool sg_fs_file_to_str(const char *path, sg_vsstr_t *out_str)
{
    FILE *fp;
    char read_buf[256];
    size_t read_size;
    size_t size;

    fp = fopen(path, "rb");
    if (!fp) {
        sg_log_err("Open file %s error.", path);
        return false;
    }

    while (!feof(fp)) {
        read_size = fread(read_buf, sizeof(char), 255, fp);
        read_buf[read_size] = 0;
        sg_vsstr_ncat(out_str, read_buf, read_size);
    }

    return true;
}

bool sg_fs_file_to_buf(const char *path, sg_vsbuf_t *out_buf)
{
    FILE *fp;
    char read_buf[256];
    size_t read_size;
    size_t size;

    fp = fopen(path, "rb");
    if (!fp) {
        sg_log_err("Open file %s error.", path);
        return false;
    }

    while (!feof(fp)) {
        read_size = fread(read_buf, sizeof(char), 256, fp);
        sg_vsbuf_insert(out_buf, read_buf, read_size);
    }

    return true;
}

bool sg_fs_file_append(const char *filename, uint8_t *data, size_t size)
{
    FILE *fp;
    size_t writed;

    fp = fopen(filename, "ab+");
    if (!fp)
        return false;

    writed = fwrite(data, 1, size, fp);
    fclose(fp);

    return (writed == size) ? true : false;
}

bool sg_fs_file_overwrite(const char *filename, uint8_t *data, size_t size)
{
    FILE *fp;
    size_t writed;

    fp = fopen(filename, "wb");
    if (!fp)
        return false;

    writed = fwrite(data, 1, size, fp);
    fclose(fp);

    return (writed == size) ? true : false;
}


/****************************************************
 * directory
 ****************************************************/

bool sg_dir_exists(const char *path)
{
    struct stat s;

    SG_ASSERT(path);
    SG_ASSERT(strlen(path) > 0);

    errno = 0;
    stat(path, &s);

    if (errno == ENOENT) /* does not exist */
        return false;

    return (S_IFDIR & s.st_mode) ? true : false; /* is directory or not */
}

bool sg_dir_seek_by_depth(const char *dir_path, uint8_t cur_depth,
                         uint8_t max_depth, sg_fs_dir_seek_cb_t cb, void *context)
{
    DIR *dir;
    sg_vsstr_t *fullpath;
    size_t dir_path_len;
    struct dirent *d;
    struct stat s;

    SG_ASSERT(dir_path);
    SG_ASSERT(strlen(dir_path) > 0);
    SG_ASSERT(cb);

    fullpath = sg_vsstr_alloc3(1024);
    if (!fullpath) {
        sg_log_err("Seek path alloc failure.");
        return false;
    }
    dir = opendir(dir_path);
    if (!dir)
        return false;
    dir_path_len = strlen(dir_path);

    while ((d = readdir(dir)) != NULL) {
        if (strcmp(d->d_name, ".") == 0) /* Ignore '.' directory. */
            continue;
        if (strcmp(d->d_name, "..") == 0) /* Ignore '..' directory. */
            continue;

        sg_vsstr_zero(fullpath);
        sg_vsstr_cat(fullpath, dir_path);
        if (dir_path[dir_path_len] != '\\' && dir_path[dir_path_len] != '/')
            sg_vsstr_cat(fullpath, "/");
        sg_vsstr_cat(fullpath, d->d_name);

#if defined(WIN32)
        DWORD ret = GetFileAttributesA(fullpath);
        if (!(ret & FILE_ATTRIBUTE_DIRECTORY))
            continue;
#else
        memset(&s, 0, sizeof(struct stat));
        lstat(sg_vsstr_raw(fullpath), &s);

        cb(SGFSDIRSEEKEVENT_FOUND, sg_vsstr_raw(fullpath), &s, context);
        if (!(S_IFDIR & s.st_mode))
            continue;
#endif
        if (max_depth == 0 || cur_depth < max_depth)
            sg_dir_seek_by_depth(sg_vsstr_raw(fullpath), cur_depth + 1,
                                 max_depth, cb, context);
    }

    closedir(dir);
    sg_vsstr_free(&fullpath);
    cb(SGFSDIRSEEKEVENT_OVER, NULL, NULL, context);

    return true;
}

bool sg_dir_seek(const char *path, uint8_t depth, sg_fs_dir_seek_cb_t cb, void *context)
{
    SG_ASSERT(path);
    SG_ASSERT(strlen(path) > 0);
    SG_ASSERT(cb);

    if (!sg_dir_exists(path)) {
        sg_log_err("Seek directory %s dose not exist.", path);
        return false;
    }

    return sg_dir_seek_by_depth(path, 1, depth, cb, context);
}