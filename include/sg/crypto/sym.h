/**
 * sym.h
 * Common symmetric encryption algorithm based on openssl,
 */

#ifndef LIBSG_SYM_H
#define LIBSG_SYM_H

#include <sg/sg.h>
#include <sg/container/vsbuf.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum sg_sym_type {
    SGSYMTYPE_AES_ECB,
    SGSYMTYPE_AES_CBC,
    SGSYMTYPE_AES_CFB128,
    SGSYMTYPE_AES_CFB1,
    SGSYMTYPE_AES_CFB8,
    SGSYMTYPE_AES_OFB,
    SGSYMTYPE_RC2_CBC,
    SGSYMTYPE_RC2_CFB,
    SGSYMTYPE_RC2_ECB,
    SGSYMTYPE_RC2_OFB,
    SGSYMTYPE_RC4,
    SGSYMTYPE_RC4_40,
    SGSYMTYPE_RC4_HMAC_MD5,
    SGSYMTYPE_RC5_32_12_16_CBC,
    SGSYMTYPE_RC5_32_12_16_ECB,
    SGSYMTYPE_RC5_32_12_16_CFB64,
    SGSYMTYPE_RC5_32_12_16_OFB,
    SGSYMTYPE_IDEA_ECB,
    SGSYMTYPE_IDEA_CFB64,
    SGSYMTYPE_IDEA_OFB,
    SGSYMTYPE_IDEA_CBC,
    SGSYMTYPE_BF_ECB,
    SGSYMTYPE_BF_CFB64,
    SGSYMTYPE_BF_OFB,
    SGSYMTYPE_BF_CBC,
    SGSYMTYPE_DES_ECB,
    SGSYMTYPE_DES_EDE,
    SGSYMTYPE_DES_EDE3,
    SGSYMTYPE_DES_EDE_ECB,
    SGSYMTYPE_DES_EDE3_ECB,
    SGSYMTYPE_DES_CFB64,
    SGSYMTYPE_DES_CFB1,
    SGSYMTYPE_DES_CFB8,
    SGSYMTYPE_DES_EDE_CFB64,
    SGSYMTYPE_DES_EDE3_CFB64,
    SGSYMTYPE_DES_EDE3_CFB1,
    SGSYMTYPE_DES_EDE3_CFB8,
    SGSYMTYPE_DES_OFB,
    SGSYMTYPE_DES_EDE_OFB,
    SGSYMTYPE_DES_EDE3_OFB,
    SGSYMTYPE_DES_CBC,
    SGSYMTYPE_DES_EDE_CBC,
    SGSYMTYPE_DES_EDE3_CBC,
};

enum sg_sym_mode {
    SGSYMMODE_ENC = 0,
    SGSYMMODE_DEC = 1
};

typedef struct sg_sym_real sg_sym_t;

typedef void (*sg_sym_on_data_func_t)(const void *data, size_t size, void *user_data);


sg_sym_t *sg_sym_alloc(enum sg_sym_type, enum sg_sym_mode mode, sg_sym_on_data_func_t cb);

void sg_sym_set_user_data(sg_sym_t *ctx, void *user_data);

int sg_sym_set_key(sg_sym_t *sym, const unsigned char *key, size_t numbits, const unsigned char* iv, size_t iv_numbits);

void sg_sym_put(sg_sym_t *, const void *data, size_t data_size);

void sg_sym_flush(sg_sym_t *);

void sg_sym_free(sg_sym_t *);

int sg_sym_str(enum sg_sym_type, const char *key, const char *iv, enum sg_sym_mode mode,
               const char *src, sg_vsbuf_t *res);

int sg_sym_buf(enum sg_sym_type type, const char *key, const char *iv, enum sg_sym_mode mode,
               const void *src, size_t src_len, sg_vsbuf_t *res);

int sg_sym_file(enum sg_sym_type, const char *key, const char *iv, enum sg_sym_mode mode,
                const char *src_filename, const char *res_filename);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_SYM_H */
