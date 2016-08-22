/**
 * big_num.h
 * Big number library based on mpir and mpfr
 */
 
/* 仅供参考，不推荐：http://blog.csdn.net/furney/article/details/7165341 */

#ifndef LIBSG_BIG_NUM_H
#define LIBSG_BIG_NUM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_big_num_real sg_big_num_t;

enum sg_big_num_cmp {
    SGBIGNUMCMP_A_BIGGER = 0, /* Left number is bigger than right. */
    SGBIGNUMCMP_EQUALS   = 1, /* Left number equals right. */
    SGBIGNUMCMP_B_BIGGER = 2  /* Right number is bigger than left. */
};

typedef enum sg_num_type {
    SGNUMTYPE_SSHORT,  /* short */
    SGNUMTYPE_USHORT,  /* unsigned short */
    SGNUMTYPE_SINT,    /* int */
    SGNUMTYPE_UINT,    /* unsigned int */
    SGNUMTYPE_SINT32,  /* int32_t */
    SGNUMTYPE_UINT32,  /* uint32_t */
    SGNUMTYPE_SINT64,  /* int64_t, long long(some platform long long equals int64_t , some not) */
    SGNUMTYPE_UINT64,  /* uint64_t */
    SGNUMTYPE_SLONG,   /* long, long int */
    SGNUMTYPE_ULONG,   /* unsigned long, unsigned long int */
    SGNUMTYPE_SFLOAT,  /* float, it is always signed */
    SGNUMTYPE_SDOUBLE, /* double, it is always signed */
    SGNUMTYPE_BIGNUM   /* integer bigger than max uint64_t or floating number bigger than max double */
}

sg_big_num_t *sg_big_num_create(void);

/* Check a string is number or not. */
bool sg_big_num_check_num_str(const char *);

/* Set common number in C such like int as value. */
int sg_big_num_set_num(sg_big_num_t *, void *num_ptr, enum sg_num_type num_type);

/* Set a number string such like "+1024.2345" as value. */
int sg_big_num_set_str(sg_big_num_t *, const char *num_str);

/* a + b = res */
int sg_big_num_add(sg_big_num_t *a, sg_big_num_t *b, sg_big_num_t *res);

/* a - b = res */
int sg_big_num_sub(sg_big_num_t *a, sg_big_num_t *b, sg_big_num_t *res);

/* a x b = res */
int sg_big_num_mul(sg_big_num_t *a, sg_big_num_t *b, sg_big_num_t *res);

/* a / b = res */
int sg_big_num_div(sg_big_num_t *a, sg_big_num_t *b, sg_big_num_t *res);

/* a % b = res */
int sg_big_num_mod(sg_big_num_t *a, sg_big_num_t *b, sg_big_num_t *res);

/* compare between a and b */
int sg_big_num_cmp(sg_big_num_t *a, sg_big_num_t *b, uint64_t max_cmp_decimal_bits_size, enum sg_big_num_cmp *res);

/* test a suitable number type, try to put it in a common number in C, but not a big number */
enum sg_num_type sg_big_num_test_suitable_type(sg_big_num_t *);

/* is positive or negative */
bool sg_bin_num_is_pos(sg_big_num_t *);

/* is integer (not int type in C) or floating number (not float type in C) */
bool sg_bin_num_is_int(sg_big_num_t *);

int sg_big_num_get_num(sg_bit_num_t *src, void *comm_num_ptr, enum sg_num_type);

int sg_big_num_get_str(sg_bit_num_t *src, uint64_t max_decimal_bits_size, sg_vlstr_t *);

void sg_big_num_destroy(sg_big_num_t *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_BIG_NUM_H */
