#include <stdint.h>
#include <string.h>
#include <sg/math/c_int.h>
#include <sg/math/big_int.h>
#include <mpir.h>

#define BIG_INT_ORDER   1
#define BIG_INT_ENDIAN  0

struct sg_big_int_real
{
    mpz_t mpz;
};

sg_big_int_t* sg_big_int_alloc(void)
{
    sg_big_int_t* num = (sg_big_int_t*) calloc(1, sizeof(sg_big_int_t));
    mpz_init(num->mpz);
    return num;
}

void sg_big_int_free(sg_big_int_t* num)
{
    mpz_clear(num->mpz);
    free(num);
}

int sg_big_int_set_bin(sg_big_int_t* dst, const unsigned char* bin, size_t bin_len)
{
    if (!dst || !bin || !bin_len)
        return -1;

    mpz_import(dst->mpz, bin_len, BIG_INT_ORDER, sizeof(unsigned char), BIG_INT_ENDIAN, 0, bin);
    return 0;
}

int sg_big_int_get_bin(sg_big_int_t* src, sg_vlbuf_t* buf)
{
    if (!src || !buf)
        return -1;

    size_t size = sizeof(unsigned char);
    size_t numb = 8 * size;
    size_t count = (mpz_sizeinbase(src->mpz, 2) + numb - 1) / numb;
    char* data = malloc (count * size);
    mpz_export(data, &count, BIG_INT_ORDER, size, BIG_INT_ENDIAN, 0, src->mpz);
    if (count > 0)
       sg_vlbuf_insert(buf, data, count); 
    free(data);
    return 0;
}

int sg_big_int_set_c_int(sg_big_int_t *dst, const void *int_ptr, enum sg_c_int_type type)
{
    if (!dst || !int_ptr)
        return -1;

    int32_t s32 = 0;
    uint32_t u32 = 0;
    int64_t s64 = 0;
    uint64_t u64 = 0;

    switch (type) {
    case SGCINTTYPE_SCHAR:
        s32 = *((char*) int_ptr);
        mpz_set_si(dst->mpz, s32);
        break;
    case SGCINTTYPE_UCHAR:
        u32 = *((unsigned char*) int_ptr);
        mpz_set_ui(dst->mpz, u32);
        break;
    case SGCINTTYPE_SSHORT:
        s32 = *((short*) int_ptr);
        mpz_set_si(dst->mpz, s32);
        break;
    case SGCINTTYPE_USHORT:
        u32 = *((unsigned short*) int_ptr);
        mpz_set_ui(dst->mpz, u32);
        break;
    case SGCINTTYPE_SINT32:
    case SGCINTTYPE_SINT:
    case SGCINTTYPE_SLONG:
        s32 = *((int32_t*) int_ptr);
        mpz_set_si(dst->mpz, s32);
        break;
    case SGCINTTYPE_UINT32:
    case SGCINTTYPE_UINT:
    case SGCINTTYPE_ULONG:
        u32 = *((uint32_t*) int_ptr);
        mpz_set_ui(dst->mpz, u32);
        break;
    case SGCINTTYPE_SINT64:
        s64 = *((int64_t*) int_ptr);
        mpz_set_sx(dst->mpz, s64);
        break;
    case SGCINTTYPE_UINT64:
        u64 = *((uint64_t*) int_ptr);
        mpz_set_ux(dst->mpz, u64);
        break;
    }
    return 0;
}

int sg_big_int_get_c_int(sg_big_int_t *src, void *int_ptr, enum sg_c_int_type type)
{
    intmax_t si;
    uintmax_t ui;

    if (!src || !int_ptr)
        return -1;

    switch (type) {
    case SGCINTTYPE_SCHAR:
    case SGCINTTYPE_SSHORT:
    case SGCINTTYPE_SINT:
    case SGCINTTYPE_SINT32:
    case SGCINTTYPE_SLONG:
    case SGCINTTYPE_SINT64:
        si = mpz_get_sx(src->mpz);
        break;
    case SGCINTTYPE_UCHAR:
    case SGCINTTYPE_USHORT:
    case SGCINTTYPE_UINT:
    case SGCINTTYPE_UINT32:
    case SGCINTTYPE_ULONG:
    case SGCINTTYPE_UINT64:
        ui = mpz_get_ux(src->mpz);
        break;
    }

    switch (type) {
    case SGCINTTYPE_SCHAR:
        *(char*) int_ptr = (char) si;
        break;
    case SGCINTTYPE_SSHORT:
        *(short*) int_ptr = (short) si;
        break;
    case SGCINTTYPE_SINT:
        *(int*) int_ptr = (int) si;
        break;
    case SGCINTTYPE_SINT32:
        *(uint32_t*) int_ptr = (uint32_t) si;
        break;
    case SGCINTTYPE_SLONG:
        *(long*) int_ptr = (long) si;
        break;
    case SGCINTTYPE_SINT64:
        *(int64_t*) int_ptr = (int64_t) si;
        break;
    case SGCINTTYPE_UCHAR:
        *(unsigned char*) int_ptr = (unsigned char) ui;
        break;
    case SGCINTTYPE_USHORT:
        *(unsigned short*) int_ptr = (unsigned short) ui;
        break;
    case SGCINTTYPE_UINT:
        *(unsigned int*) int_ptr = (unsigned int) ui;
        break;
    case SGCINTTYPE_UINT32:
        *(uint32_t*) int_ptr = (uint32_t) ui;
        break;
    case SGCINTTYPE_ULONG:
        *(unsigned long*) int_ptr = (unsigned long) ui;
        break;
    case SGCINTTYPE_UINT64:
        *(uint64_t*) int_ptr = (uint64_t) ui;
        break;
    }
    return 0;
}

int sg_big_int_set_c_float(sg_big_int_t *dst, const void *c_float_ptr, enum sg_c_float_type type)
{
    if (!dst || !c_float_ptr)
        return -1;

    double d = 0;
    switch (type) {
    case SGCFLOATTYPE_SFLOAT:
        d = (*(float*) c_float_ptr);
        mpz_set_d(dst->mpz, d);
        break;
    case SGCFLOATTYPE_SDOUBLE:
        d = (*(double*) c_float_ptr);
        mpz_set_d(dst->mpz, d);
        break;
    }
    mpz_set_d(dst->mpz, d);
    return 0;
}

int sg_big_int_get_c_float(sg_big_int_t *src, void *c_float_ptr, enum sg_c_float_type type)
{
    if (!src || !c_float_ptr)
        return -1;

    double d = mpz_get_d(src->mpz);
    switch (type) {
    case SGCFLOATTYPE_SFLOAT:
        *(float*) c_float_ptr = (float) d;
        break;
    case SGCFLOATTYPE_SDOUBLE:
        *(double*) c_float_ptr = d;
        break;
    }
    return 0;
}

int sg_big_int_set_big_int(sg_big_int_t *dst, sg_big_int_t *src)
{
    if (!dst || !src)
        return -1;

    mpz_set(dst->mpz, src->mpz);
    return 0;
}

int sg_big_int_set_big_float(sg_big_int_t *dst, sg_big_float_t *src)
{
    /* Trunc the float number to be integer in order to get a float-string without exponent,
       which can pass to sg_big_int_set_str directly. */
    sg_big_float_trunc(src, src);
    sg_vlstr_t* str = sg_vlstralloc();
    size_t exponent = 0;
    sg_big_float_get_mantissa_and_exponent(src, SGNUMSYS_DEC, str, &exponent);

    size_t padding = exponent;
    const char* p = sg_vlstrraw(str);
    /* count up negative symbol */
    if (p[0] == '-')
        ++padding;
    padding -= strlen(p);
    for (size_t i = 0; i < padding; ++i)
        sg_vlstrcat(str, "0");

    sg_big_int_set_str(dst, sg_vlstrraw(str), SGNUMSYS_DEC);
    sg_vlstrfree(&str);
    return 0;
}

int sg_big_int_set_str(sg_big_int_t *dst, const char *num_str, enum sg_num_sys sys)
{
    int base = SG_COMPUTE_BASE(sys);
    if (!dst || !num_str || base <= 0)
        return -1;

    mpz_set_str(dst->mpz, num_str, base);
    return 0;
}

int sg_big_int_get_str(sg_big_int_t *src, enum sg_num_sys sys, sg_vlstr_t * str)
{
    int base = SG_COMPUTE_BASE(sys);
    if (!src || !str || base <= 0)
        return -1;

    char* s = (char*) malloc(mpz_sizeinbase(src->mpz, base) + 2);
    mpz_get_str(s, base, src->mpz);
    sg_vlstrcpy(str, s);
    free(s);
    return 0;
}

int sg_big_int_add(sg_big_int_t *a, sg_big_int_t *b, sg_big_int_t *res)
{
    if (!a || !b || !res)
        return -1;

    mpz_add(res->mpz, a->mpz, b->mpz);
    return 0;
}

int sg_big_int_sub(sg_big_int_t *a, sg_big_int_t *b, sg_big_int_t *res)
{
    if (!a || !b || !res)
        return -1;

    mpz_sub(res->mpz, a->mpz, b->mpz);
    return 0;
}

int sg_big_int_mul(sg_big_int_t *a, sg_big_int_t *b, sg_big_int_t *res)
{
    if (!a || !b || !res)
        return -1;

    mpz_mul(res->mpz, a->mpz, b->mpz);
    return 0;
}

int sg_big_int_div(sg_big_int_t *a, sg_big_int_t *b, sg_big_int_t *res)
{
    if (!a || !b || !res)
        return -1;

    mpz_cdiv_q(res->mpz, a->mpz, b->mpz);
    return 0;
}

int sg_big_int_mod(sg_big_int_t *a, sg_big_int_t *b, sg_big_int_t *res)
{
    if (!a || !b || !res)
        return -1;

    mpz_mod(res->mpz, a->mpz, b->mpz);
    return 0;
}

int sg_big_int_sqrt(sg_big_int_t *a, sg_big_int_t *res)
{
    if (!a || !res)
        return -1;

    mpz_sqrt(res->mpz, a->mpz);
    return 0;
}

int sg_big_int_shift(sg_big_int_t *a, bool left_shift, size_t n, sg_big_int_t *res)
{
    if (!a || !n || !res)
        return -1;

    /* FIXME: using multiply and divide instead of shift, try to find a better solution */
    if (left_shift)
        mpz_mul_2exp(res->mpz, a->mpz, n);
    else
        mpz_cdiv_q_2exp(res->mpz, a->mpz, n);

    return 0;
}

enum sg_big_int_cmp_result sg_big_int_cmp(sg_big_int_t *a, sg_big_int_t *b)
{
    int ret = mpz_cmp(a->mpz, b->mpz);
    return ret > 0 ? SGBIGINTCMP_A_BIGGER : ret == 0 ? SGBIGINTCMP_EQUALS : SGBIGINTCMP_B_BIGGER;
}

int sg_big_int_cmp_zero(sg_big_int_t* a)
{
    return mpz_cmp_ui(a->mpz, 0);
}

int sg_big_int_try_to_get_c_int_type(sg_big_int_t *src, enum sg_c_int_type *type)
{
    if (!src || !type)
        return -1;

    if (mpz_fits_sshort_p(src->mpz))
        *type = SGCINTTYPE_SSHORT;
    else if (mpz_fits_ushort_p(src->mpz))
        *type = SGCINTTYPE_USHORT;
    else if (mpz_fits_sint_p(src->mpz))
        *type = SGCINTTYPE_SINT;
    else if (mpz_fits_uint_p(src->mpz))
        *type = SGCINTTYPE_UINT;
    else if (mpz_fits_slong_p(src->mpz))
        *type = SGCINTTYPE_SLONG;
    else if (mpz_fits_ulong_p(src->mpz))
        *type = SGCINTTYPE_ULONG;

    return 0;
}

bool sg_big_int_check_num_str(const char *int_str, enum sg_num_sys sys)
{
    mpz_t z;
    int base = SG_COMPUTE_BASE(sys);

    if (!int_str || !base)
        return false;

    return mpz_init_set_str(z, int_str, base) == 0;
}

