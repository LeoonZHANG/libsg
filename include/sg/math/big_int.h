/**
 * big_int.h
 * Big integer and floating-pointer number library based on mpir.
 */

/* 如果头文件中有设计不合理的地方，请酌情调整 */
/* res可以等于a或者b，这样避免开辟太多的变量 */

#ifndef LIBSG_BIG_INT_H
#define LIBSG_BIG_INT_H

#include <sg/sg.h>
#include <sg/container/vsbuf.h>
#include <sg/str/vsstr.h>
#include "c_int.h"
#include "num_sys.h"
#include "big_float.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_big_int_real sg_big_int_t;
typedef struct sg_big_float_real sg_big_float_t;

enum sg_big_int_cmp_result {
    SGBIGINTCMP_A_BIGGER = 0, /* Left number is bigger than right. */
    SGBIGINTCMP_EQUALS   = 1, /* Left number equals right. */
    SGBIGINTCMP_B_BIGGER = 2  /* Right number is bigger than left. */
};

sg_big_int_t *sg_big_int_alloc(void);

int sg_big_int_set_bin(sg_big_int_t* dst, const unsigned char* bin, size_t bin_len);

/* Set number such like int32_t / sg_big_int_t / sg_big_float_t as value. */
/* dst = c_num */
int sg_big_int_set_c_int(sg_big_int_t *dst, const void *c_int_ptr, enum sg_c_int_type type);
int sg_big_int_set_c_float(sg_big_int_t *dst, const void *c_float_ptr, enum sg_c_float_type type);
int sg_big_int_set_big_int(sg_big_int_t *dst, sg_big_int_t *src);
int sg_big_int_set_big_float(sg_big_int_t *dst, sg_big_float_t *src);

/* Set a number string such like "+1024" as value. */
int sg_big_int_set_str(sg_big_int_t *dst, const char *num_str, enum sg_num_sys sys);

/* a + b = res */
int sg_big_int_add(sg_big_int_t *a, sg_big_int_t *b, sg_big_int_t *res);

/* a - b = res */
int sg_big_int_sub(sg_big_int_t *a, sg_big_int_t *b, sg_big_int_t *res);

/* a x b = res */
int sg_big_int_mul(sg_big_int_t *a, sg_big_int_t *b, sg_big_int_t *res);

/* a / b = res */
int sg_big_int_div(sg_big_int_t *a, sg_big_int_t *b, sg_big_int_t *res);

/* a % b = res */
int sg_big_int_mod(sg_big_int_t *a, sg_big_int_t *b, sg_big_int_t *res);

/* sqrt(a) = res */
int sg_big_int_sqrt(sg_big_int_t *a, sg_big_int_t *res);

/* if (is_left_shift) { a << n = res } else { a >> n = res } */
int sg_big_int_shift(sg_big_int_t *a, bool left_shift, size_t n, sg_big_int_t *res);

/* compare between a and b */
enum sg_big_int_cmp_result sg_big_int_cmp(sg_big_int_t *a, sg_big_int_t *b);

/* return: ＋1 positive, ==0 zero, －1 negative, otherwise error */
int sg_big_int_cmp_zero(sg_big_int_t *);

/* try to find a suitable C integer type, and put it in it */
int sg_big_int_try_to_get_c_int_type(sg_big_int_t *src, enum sg_c_int_type *type);

/* Check a string is number or not. */
bool sg_big_int_check_num_str(const char *int_str, enum sg_num_sys sys);

int sg_big_int_get_bin(sg_big_int_t *src, sg_vsbuf_t *);
int sg_big_int_get_c_int(sg_big_int_t *src, void *c_int_ptr, enum sg_c_int_type);
int sg_big_int_get_c_float(sg_big_int_t *src, void *c_float_ptr, enum sg_c_float_type);
int sg_big_int_get_str(sg_big_int_t *src, enum sg_num_sys sys, sg_vsstr_t *);

void sg_big_int_free(sg_big_int_t *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_BIG_INT_H */
