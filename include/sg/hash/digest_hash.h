/*
 * digest_hash.
 * Common encrypt hash tool package based on openssl,
 * including (hmac) md5 / sha1 / sha256 / sha512 / rc4.
 */

#ifndef LIBSG_DIGEST_HASH_H
#define LIBSG_DIGEST_HASH_H

#include <stdint.h> /* uint8_t */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum sg_digest_hash_type {
    SGDIGESTHASHTYPE_MD5         = 0,
    SGDIGESTHASHTYPE_HMAC_MD5    = 1,
    SGDIGESTHASHTYPE_SHA1        = 2,
    SGDIGESTHASHTYPE_HMAC_SHA1   = 3,
    SGDIGESTHASHTYPE_SHA256      = 4,
    SGDIGESTHASHTYPE_HAMC_SHA256 = 5,
    SGDIGESTHASHTYPE_SHA512      = 6,
    SGDIGESTHASHTYPE_HMAC_SHA512 = 7
}

/* 如果定义不对，请修改 */
#define SGDIGESTHASHRAWBIN_MAXLEN 64
#define SGDIGESTHASHHEXSTR_MAXLEN 129 /* 128 + terminator，sha512的16进制字符哈希值最长，为128 */

/* hash value.
   If all characters are zero, that means error returned. */
struct sg_digest_hash_sum {
    size_t  raw_bin_used_len;
    uint8_t raw_bin[SGENCHASHRAWBIN_MAXLEN]; /* Output hash as a raw binary format data. */
    char    hex_str[SGENCHASHHEXSTR_MAXLEN]; /* Output hash as a hexadecimal number. */
};

/* Md5 handle opened by md5_open. */
typedef void sg_digest_hash_ctx;

/* Md5 hash for a binary buffer.
   Length must > 0. */
int sg_digest_hash_buf(void *buf, size_t len, enum sg_digest_hash_type type,
        const char *hmac_key, struct sg_digest_hash_sum *rst);

/* Md5 hash for a c plain string.
   String length must > 0. */
int sg_digest_hash_str(const char *str, enum sg_digest_hash_type type,
        const char *hmac_key, struct sg_digest_hash_sum *rst);

/* Md5 hash for a file.
   Length of path must > 0. */
int sg_digest_hash_file(const char *path, enum sg_digest_hash_type type,
        const char *hmac_key, struct sg_digest_hash_sum *rst);

/* Open a md5 context. */
sg_digest_hash_ctx *sg_digest_hash_start(enum sg_digest_hash_type type,
        const char *hmac_key);

/* Update data to hash it.
   Length must > 0. */
int sg_digest_hash_update(sg_digest_hash_ctx *ctx, void *input, size_t input_len);

/* Close the md5 context and get the result. */
int sg_digest_hash_finish(sg_digest_hash_ctx *ctx, struct sg_digest_hash_sum *rst);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_DIGEST_HASH_H */