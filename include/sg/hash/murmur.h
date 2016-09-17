/**
 * murmur.h
 * murmur hash.
 */

/**
 * 基于 https://github.com/PeterScott/murmur3
 */
#ifndef LIBSG_MURMUR_H
#define LIBSG_MURMUR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SG_MURMUR_HASH_HEX_LEN ?

enum sg_murmur_type {
    SGMURMURTYPE_1 = 0, /* murmur 1 */
    SGMURMURTYPE_2 = 1, /* murmur 2 */
    SGMURMURTYPE_3 = 2  /* murmur 3 */
};

typedef struct sg_murmur_ctx_real sg_murmur_ctx;


struct sg_murmur_sum {
    char SG_MURMUR_HASH_HEX_LEN[?];
};


typedef void (*sg_murmur_on_data_func_t)(const void *data, size_t size, void *user_data);


int sg_murmur_buf(enum sg_murmur_type, const void *buf, size_t len, struct sg_murmur_sum *);

int sg_murmur_str(enum sg_murmur_type, const char *str, size_t len, struct sg_murmur_sum *);

int sg_murmur_file(enum sg_murmur_type, const char *path, struct sg_murmur_sum *);




sg_murmur_t *sg_murmur_start(enum sg_murmur_type);

int sg_murmur_update(sg_murmur_t *self, void *data, uint64_t size);

int sg_murmur_finish(sg_murmur_t *self, struct sg_murmur_sum *);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_MURMUR_H */