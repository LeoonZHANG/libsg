/**
 * big_float.h
 * Big integer and floating-pointer number library based on mpir.
 */

/* 如果头文件中有设计不合理的地方，请酌情调整 */
/* res可以等于a或者b，这样避免开辟太多的变量 */

#ifndef LIBSG_BIG_FLOAT_H
#define LIBSG_BIG_FLOAT_H

#include "vlbuf.h"
#include "c_num.h"
#include "num_sys.h"
#include "big_int.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_big_float_real sg_big_float_t;

enum sg_big_float_cmp {
    SGBIGFLOATCMP_A_BIGGER = 0, /* Left number is bigger than right. */
    SGBIGFLOATCMP_EQUALS   = 1, /* Left number equals right. */
    SGBIGFLOATCMP_B_BIGGER = 2  /* Right number is bigger than left. */
};

sg_big_float_t *sg_big_float_alloc(void);

int sg_big_float_set_bin(sg_big_float_t *dst, const unsigned char *bin, int bin_len);

/* Set number such like float / sg_big_float_t / sg_big_float_t as value. */
/* dst = c_num */
int sg_big_float_set_c_int(sg_big_float_t *dst, const void *c_int_ptr, enum sg_c_int_type type);
int sg_big_float_set_c_float(sg_big_float_t *dst, const void *c_float_ptr, enum sg_c_float_type type);
int sg_big_float_set_big_int(sg_big_float_t *dst, sg_big_float_t *src);
int sg_big_float_set_big_float(sg_big_float_t *dst, sg_big_float_t *src);
int sg_big_float_set_str(sg_big_float_t *dst, const char *num_str, enum sg_num_sys sys);

/* a + b = res */
int sg_big_float_add(sg_big_float_t *a, sg_big_float_t *b, sg_big_float_t *res);

/* a - b = res */
int sg_big_float_sub(sg_big_float_t *a, sg_big_float_t *b, sg_big_float_t *res);

/* a x b = res */
int sg_big_float_mul(sg_big_float_t *a, sg_big_float_t *b, sg_big_float_t *res);

/* a / b = res */
int sg_big_float_div(sg_big_float_t *a, sg_big_float_t *b, sg_big_float_t *res);

/* a % b = res */
int sg_big_float_mod(sg_big_float_t *a, sg_big_float_t *b, sg_big_float_t *res);

/* sqrt(a) = res */
int sg_big_float_sqrt(sg_big_float_t *a, sg_big_float_t *res);

/* compare between a and b */
int sg_big_float_cmp(sg_big_float_t *a, sg_big_float_t *b, enum sg_big_float_cmp *res);

/* return: ＋1 positive, ==0 zero, －1 negative, otherwise error */
int sg_big_float_cmp_zero(sg_big_float_t *src);

/* Check a string is number or not. */
bool sg_big_float_check_num_str(const char *int_str, enum sg_num_sys sys);

/* try to find a suitable C floating-pointer type, and put it in it */
int sg_big_float_try_to_get_c_float_type(sg_big_float_t *src, enum sg_c_float_type *type);

int sg_big_float_get_bin(sg_bit_int_t *src, sg_vlbuf_t *);
int sg_big_float_get_c_int(sg_bit_int_t *src, void *c_int_ptr, enum sg_c_int_type);
int sg_big_float_get_c_float(sg_bit_int_t *src, void *c_float_ptr, enum sg_c_float_type);
int sg_big_float_get_str(sg_bit_int_t *src, enum sg_num_sys sys, sg_vlstr_t *);

void sg_big_float_free(sg_big_float_t *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_BIG_FLOAT_H */