/*
 * file.c
 * Author: wangwei.
 * Regular file and directory common operations package.
 */

#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sg/sys/os.h>
#if defined(OS_WIN)
# include <sg/platform/windows/dirent.h>
#endif
#if defined(OS_LNX)
# include <bits/errno.h>
# include <dirent.h>
# include <stdint.h>
# include <stdint-gcc.h>
#endif
#if defined(OS_OSX)
# include <dirent.h>
#endif
#include <sg/sys/file.h>
#include <sg/util/def.h>
#include <sg/util/assert.h>
#include <sg/str/string.h>
#include <sg/util/log.h>

int dir_seek_by_depth(const char *dir_path, uint8_t cur_depth,
                      uint8_t max_depth, sg_dir_seek_callback cb, void *context);

int sg_path_exists(const char *path)
{
    struct stat s;

    assert(path);
    assert(strlen(path) > 0);

    errno = 0;
    stat(path, &s);

    return (errno == ENOENT) ? 0 : 1; /* does not exist */
}

bool sg_reg_file_exists(const char *path)
{
    struct stat s;

    assert(path);
    assert(strlen(path) > 0);

    errno = 0;
    stat(path, &s);

    if (errno == ENOENT) /* does not exist */
        return false;

    return (S_IFREG & s.st_mode) ? true : false; /* is regular file or not */
}

long sg_reg_file_size(const char *path)
{
    struct stat s;

    assert(path);
    assert(strlen(path) > 0);

    errno = 0;
    stat(path, &s);

    if (errno == ENOENT) /* does not exist */
        return -1;

    if (!(S_IFREG & s.st_mode)) /* is regular file or not */
        return -1;

    return s.st_size;
}

int sg_reg_file_remove(const char *path)
{
    assert(path);

    return remove(path);
}

int sg_dir_exists(const char *path)
{
    struct stat s;

    assert(path);
    assert(strlen(path) > 0);

    errno = 0;
    stat(path, &s);

    if (errno == ENOENT) /* does not exist */
        return 0;

    return (S_IFDIR & s.st_mode) ? 1 : 0; /* is directory or not */
}

int sg_dir_seek_by_depth(const char *dir_path, uint8_t cur_depth,
                         uint8_t max_depth, sg_dir_seek_callback cb, void *context)
{
    DIR *dir;
    sg_vlstr_t *fullpath;
    size_t dir_path_len;
    struct dirent *d;
    struct stat s;

    assert(dir_path);
    assert(strlen(dir_path) > 0);
    assert(cb);

    fullpath = sg_vlstralloc3(1024);
    if (!fullpath) {
        sg_log_err("Seek path alloc failure.");
        return -1;
    }
    dir = opendir(dir_path);
    if (!dir)
        return -1;
    dir_path_len = strlen(dir_path);

    while ((d = readdir(dir)) != NULL) {
        if (strcmp(d->d_name, ".") == 0) /* Ignore '.' directory. */
            continue;
        if (strcmp(d->d_name, "..") == 0) /* Ignore '..' directory. */
            continue;

        sg_vlstrempty(fullpath);
        sg_vlstrcat(fullpath, dir_path);
        if (dir_path[dir_path_len] != '\\' && dir_path[dir_path_len] != '/')
            sg_vlstrcat(fullpath, "/");
        sg_vlstrcat(fullpath, d->d_name);

#if defined(WIN32)
        DWORD ret = GetFileAttributesA(fullpath);
        if (!(ret & FILE_ATTRIBUTE_DIRECTORY))
            continue;
#else
        ZERO(s, struct stat);
        lstat(sg_vlstrraw(fullpath), &s);

        cb(SGDIRSEEKEVENT_FOUND, sg_vlstrraw(fullpath), &s, context);
        if (!(S_IFDIR & s.st_mode))
            continue;
#endif
        if (max_depth == 0 || cur_depth < max_depth)
            sg_dir_seek_by_depth(sg_vlstrraw(fullpath), cur_depth + 1,
                                 max_depth, cb, context);
    }

    closedir(dir);
    sg_vlstrfree(&fullpath);
    cb(SGDIRSEEKEVENT_OVER, NULL, NULL, context);

    return 0;
}

int sg_dir_seek(const char *dir_path, uint8_t depth, sg_dir_seek_callback cb, void *context)
{
    assert(dir_path);
    assert(strlen(dir_path) > 0);
    assert(cb);

    if (sg_dir_exists(dir_path) == 0) {
        sg_log_err("Seek directory %s dose not exist.", dir_path);
        return -1;
    }

    return sg_dir_seek_by_depth(dir_path, 1, depth, cb, context);
}

sg_vlstr_t *sg_file_get_ext(const char *filename, int uppercase)
{
    sg_vlstr_t *v;
    char *dot;

    assert(filename);

    dot = strrchr((char *)filename, '.');
    if (!dot)
        return NULL;
    /* Dot is the last character before terminator. */
    if (dot == filename + strlen(filename) - 1)
        return NULL;

    v = sg_vlstrdup(dot);
    if (!v || !sg_vlstrraw(v))
        return NULL;

    if (uppercase == 0)
        sg_str_to_lower(sg_vlstrraw(v));
    else
        sg_str_to_upper(sg_vlstrraw(v));

    return v;
}

sg_vlstr_t *sg_file_to_str(const char *filename)
{
    FILE *fp;
    size_t size;
    sg_vlstr_t *retval;
    char *raw;

    fp = fopen(filename, "rb");
    if (!fp) {
        sg_log_err("Open file %s error.", filename);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    size = (size_t)ftell(fp);
    if (size == 0)
        return sg_vlstralloc();

    retval = sg_vlstralloc3(size + 1);
    if (!retval) {
        sg_log_err("retval alloc error.");
        return NULL;
    }

    raw = sg_vlstrraw(retval);
    fseek(fp, 0, SEEK_SET);
    while (!feof(fp))
        fread(raw, sizeof(char), size, fp);
    raw[size] = '\0'; /* terminator */

    return retval;
}

int sg_file_to_buf(const char *filename, uint8_t **buffer, size_t *size)
{
    FILE *fp;
    size_t read_size = 0;

    sg_assert(filename);
    sg_assert(buffer);
    sg_assert(size);

    fp = fopen(filename, "rb");
    if (!fp) {
        sg_log_err("Open file %s error.", filename);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    *size = (size_t)ftell(fp);
    if (*size <= 0) {
        *buffer = NULL;
        return 0;
    }

    *buffer = (uint8_t *)malloc(*size);
    if (!*buffer) {
        sg_log_err("malloc error.");
        return -1;
    }

    rewind(fp); /* fseek(fp, 0, SEEK_SET); */
    while (!feof(fp))
        read_size += fread((void *)*buffer, sizeof(uint8_t), *size, fp);
    return 0;
}

int sg_file_append(const char *filename, uint8_t *data, size_t size)
{
    FILE *fp;
    size_t writed;

    fp = fopen(filename, "ab+");
    if (!fp)
        return -1;

    writed = fwrite(data, 1, size, fp);
    fclose(fp);

    return (writed == size) ? 0 : -1;
}

int sg_file_overwrite(const char *filename, uint8_t *data, size_t size)
{
    FILE *fp;
    size_t writed;

    fp = fopen(filename, "wb");
    if (!fp)
        return -1;

    writed = fwrite(data, 1, size, fp);
    fclose(fp);

    return (writed == size) ? 0 : -1;
}
