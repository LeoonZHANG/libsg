/**
 * mac.h
 * Common message authentication code tools package based on openssl.
 */

#ifndef LIBSG_MAC_H
#define LIBSG_MAC_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum sg_mac_type {
    SGMACTYPE_HMAC_MD2    = 0,
    SGMACTYPE_HMAC_MD4    = 1,
    SGMACTYPE_HMAC_MD5    = 2,
    SGMACTYPE_HMAC_SHA1   = 3,
    SGMACTYPE_HMAC_SHA224 = 4,
    SGMACTYPE_HMAC_SHA256 = 5,
    SGMACTYPE_HMAC_SHA384 = 6,
    SGMACTYPE_HMAC_SHA512 = 7
};

/* 如果定义不对，请修改 */
#define SGMACRAWBIN_MAXLEN 64
#define SGMACHEXSTR_MAXLEN 129 /* 128 + terminator，sha512的16进制字符哈希值最长，为128 */

/* hash value.
   If all characters are zero, that means error returned. */
struct sg_mac_sum {
    uint8_t raw_bin[SGMACRAWBIN_MAXLEN]; /* Output hash as a raw binary format data. */
    char    hex_str[SGMACHEXSTR_MAXLEN]; /* Output hash as a hexadecimal number. */
    unsigned int raw_bin_used_len;
};

/* Mac handle opened by mac5_open. */
typedef struct sg_mac sg_mac_ctx;

/* Mac hash for a binary buffer.
   Length must > 0. */
int sg_mac_buf(const void *buf, size_t len, enum sg_mac_type type, const char *key, struct sg_mac_sum *rst);

/* Mac for a c plain string.
   String length must > 0. */
int sg_mac_str(const char *str, enum sg_mac_type type,
               const char *key, struct sg_mac_sum *rst);

/* Mac hash for a file.
   Length of path must > 0. */
int sg_mac_file(const char *path, enum sg_mac_type type,
                const char *key, struct sg_mac_sum *rst);

/* Open a mac context. */
sg_mac_ctx *sg_mac_start(enum sg_mac_type type, const char *key);

/* Update data to hash it.
   Length must > 0. */
int sg_mac_update(sg_mac_ctx *mac, const void *input, size_t input_len);

/* Close the mac context and get the result. */
int sg_mac_finish(sg_mac_ctx *ctx, struct sg_mac_sum *rst);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_MAC_H */
