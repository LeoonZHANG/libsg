/*
 * md5.h
 * Author: wangwei.
 * Md5 hash tool based on the work of Alexander Peslyak and WaterJuice.
 */

#ifndef LIBSG_MD5_H
#define LIBSG_MD5_H

#include <stdint.h> /* uint8_t */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Md5 hash output formats. */
enum sg_md5_fmt {
    /* Output md5 hash as a raw binary format data with a length of 16. */
    SGMD5FMT_RAW = 0,
    /* Output md5 hash as a 32-character hexadecimal number. */
    SGMD5FMT_STR = 1,
};

/* Md5 hash value.
   If all characters are zero, that means error returned. */
struct sg_md5_hash {
    uint8_t buf[33];
};

/* Md5 handle opened by md5_open. */
typedef void sg_md5_ctx;

/* Md5 hash for a binary buffer.
   Length must > 0. */
struct sg_md5_hash sg_md5_easy_buf(void *buf, size_t len, enum sg_md5_fmt fmt);

/* Md5 hash for a c plain string.
   String length must > 0. */
struct sg_md5_hash sg_md5_easy_str(const char *str, enum sg_md5_fmt fmt);

/* Md5 hash for a file.
   Length of path must > 0. */
struct sg_md5_hash sg_md5_easy_file(const char *path, enum sg_md5_fmt fmt);

/* Open a md5 context. */
sg_md5_ctx *sg_md5_start(void);

/* Update data to hash it.
   Length must > 0. */
void sg_md5_update(sg_md5_ctx *mc, void *buf, size_t len);

/* Close the md5 context and get the result. */
struct sg_md5_hash sg_md5_finish(sg_md5_ctx *mc, enum sg_md5_fmt fmt);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_MD5_H */