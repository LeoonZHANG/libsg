/**
 * md.h
 * Common message digest hash tools package based on openssl,
 * including md2 / md4 / md5 / sha1 / sha224 / sha256 / sha384 / sha512 / RIPEMD.
 */

#ifndef LIBSG_MD_H
#define LIBSG_MD_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum sg_md_type {
    SGMDTYPE_MD2         = 0,
    SGMDTYPE_MD4         = 1,
    SGMDTYPE_MD5         = 2,
    SGMDTYPE_SHA1        = 3,
    SGMDTYPE_SHA224      = 4,
    SGMDTYPE_SHA256      = 5,
    SGMDTYPE_SHA384      = 6,
    SGMDTYPE_SHA512      = 7,
    SGMDTYPE_RIPEMD      = 8
};

/* 如果定义不对，请修改 */
#define SGMDRAWBIN_MAXLEN 64
#define SGMDHEXSTR_MAXLEN 129 /* 128 + terminator，sha512的16进制字符哈希值最长，为128 */

/* hash value.
   If all characters are zero, that means error returned. */
struct sg_md_sum {
    size_t  raw_bin_used_len;
    uint8_t raw_bin[SGMDRAWBIN_MAXLEN]; /* Output hash as a raw binary format data. */
    char    hex_str[SGMDHEXSTR_MAXLEN]; /* Output hash as a hexadecimal number. */
};

/* Message digest handle opened by sg_md_start. */
typedef struct sg_md_ctx sg_md_t;

/* Message digest hash for a binary buffer.
   Length must > 0. */
int sg_md_buf(const void *buf, size_t len, enum sg_md_type type, struct sg_md_sum *rst);

/* Message digest hash for a c plain string.
   String length must > 0. */
int sg_md_str(const char *str, enum sg_md_type type, struct sg_md_sum *rst);

/* Message digest hash for a file.
   Length of path must > 0. */
int sg_md_file(const char *path, enum sg_md_type type, struct sg_md_sum *rst);

/* Open a message digest context. */
sg_md_t *sg_md_start(enum sg_md_type type);

/* Update data to hash it.
   Length must > 0. 填入数据大小不限制,如果算法接口限制了每次接口调用时传入数据的大小,请在此模块内部解决这个差异。 */
int sg_md_update(sg_md_t *ctx, const void *buf, size_t len);

/* Close the message digest context and get the result. */
int sg_md_finish(sg_md_t *ctx, struct sg_md_sum *rst);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_MD_H */
