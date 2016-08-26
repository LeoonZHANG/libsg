/**
 * sym.h
 * Common symmetric encryption algorithm based on openssl, including DES / 3DES / AES / IDEA / RC2 / RC4 / RC5 / BlowFish等等
 * 这个模块整合的对称加密算法中可以把输入输出抽象的如头文件接口设计等那些对称加密算法,如果有些对称加密算法确定不可以这样抽象,那么就不要放这个模块
 * enum sg_sym_type请补齐
 */

#ifndef LIBSG_SYM_H
#define LIBSG_SYM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum sg_sym_type {
    SGSYMTYPE_AES        = 0,
    SGSYMTYPE_AES_ECB    = 1,
    SGSYMTYPE_AES_CBC    = 2,
    SGSYMTYPE_AES_CFB128 = 3,
    SGSYMTYPE_AES_CFB1   = 4,
    SGSYMTYPE_AES_CFB8   = 5,
    SGSYMTYPE_AES_OFB128 = 6,
    SGSYMTYPE_AES_IGE    = 7,
    SGSYMTYPE_AES_BI_IGE = 8,
};

enum sg_sym_mode {
    SGSYMMODE_ENC = 0,
    SGSYMMODE_DEC = 1
};

typedef struct sg_sym_real sg_sym_t;

typedef void (*sg_sym_on_data_func_t)(void *data, size_t size, void *user_data);


sg_sym_t *sg_sym_alloc(enum sg_sym_type, enum sg_sym_mode mode, sg_sym_on_data_func_t cb);

void sg_sym_set_user_data(sg_sym_t *ctx, void *user_data);

int sg_sym_set_key(sg_sym_t *ctx, const unsigned char *key, size_t key_bits);

void sg_sym_put(sg_sym_t *, void *data, size_t data_size);

void sg_sym_flush(sg_sym_t *);

void sg_sym_free(sg_sym_t *);

int sg_sym_str(enum sg_sym_type, const char *key, enum sg_sym_mode mode,
               const char *src, sg_vlstr_t *res);

int sg_sym_buf(enum sg_sym_type, const char *key, enum sg_sym_mode mode,
               const void *src, size_t src_len, sg_vlbuf_t *res);

int sg_sym_file(enum sg_sym_type, const char *key, enum sg_sym_mode mode,
                const char *src_fn, const char *res_fn);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_SYM_H */