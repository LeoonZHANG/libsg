#include <string.h>
#include <mpir.h>
#include <sg/sg.h>
#include <sg/math/c_float.h>
#include <sg/math/big_float.h>
#include <sg/math/big_int.h>

#define BIG_FLOAT_ORDER   1
#define BIG_FLOAT_ENDIAN  0

struct sg_big_float_real
{
    mpf_t mpf;
};

sg_big_float_t* sg_big_float_alloc(void)
{
    sg_big_float_t* num = (sg_big_float_t*) calloc(1, sizeof(sg_big_float_t));
    mpf_init(num->mpf);
    return num;
}

void sg_big_float_free(sg_big_float_t* num)
{
    mpf_clear(num->mpf);
    free(num);
}

int sg_big_float_set_c_int(sg_big_float_t *dst, const void *c_int_ptr, enum sg_c_int_type type)
{
    if (!dst || !c_int_ptr)
        return -1;

    int32_t s32 = 0;
    uint32_t u32 = 0;
    double d = 0;

    switch (type) {
    case SGCINTTYPE_SCHAR:
        s32 = *((char*) c_int_ptr);
        mpf_set_si(dst->mpf, s32);
        break;
    case SGCINTTYPE_UCHAR:
        u32 = *((unsigned char*) c_int_ptr);
        mpf_set_ui(dst->mpf, u32);
        break;
    case SGCINTTYPE_SSHORT:
        s32 = *((short*) c_int_ptr);
        mpf_set_si(dst->mpf, s32);
        break;
    case SGCINTTYPE_USHORT:
        u32 = *((unsigned short*) c_int_ptr);
        mpf_set_ui(dst->mpf, u32);
        break;
    case SGCINTTYPE_SINT32:
    case SGCINTTYPE_SINT:
    case SGCINTTYPE_SLONG:
        s32 = *((int32_t*) c_int_ptr);
        mpf_set_si(dst->mpf, s32);
        break;
    case SGCINTTYPE_UINT32:
    case SGCINTTYPE_UINT:
    case SGCINTTYPE_ULONG:
        u32 = *((uint32_t*) c_int_ptr);
        mpf_set_ui(dst->mpf, u32);
        break;
    case SGCINTTYPE_SINT64:
        d = *((int64_t*) c_int_ptr);
        mpf_set_d(dst->mpf, d);
        break;
    case SGCINTTYPE_UINT64:
        d = *((uint64_t*) c_int_ptr);
        mpf_set_d(dst->mpf, d);
        break;
    }
    return 0;
}

int sg_big_float_get_c_int(sg_big_float_t *src, void *int_ptr, enum sg_c_int_type type)
{
    double d;

    if (!src || !int_ptr)
        return -1;

    d = mpf_get_d(src->mpf);

    switch (type) {
    case SGCINTTYPE_SCHAR:
        *(char*) int_ptr = (char) d;
        break;
    case SGCINTTYPE_SSHORT:
        *(short*) int_ptr = (short) d;
        break;
    case SGCINTTYPE_SINT:
        *(int*) int_ptr = (int) d;
        break;
    case SGCINTTYPE_SINT32:
        *(uint32_t*) int_ptr = (uint32_t) d;
        break;
    case SGCINTTYPE_SLONG:
        *(long*) int_ptr = (long) d;
        break;
    case SGCINTTYPE_SINT64:
        *(int64_t*) int_ptr = (int64_t) d;
        break;
    case SGCINTTYPE_UCHAR:
        *(unsigned char*) int_ptr = (unsigned char) d;
        break;
    case SGCINTTYPE_USHORT:
        *(unsigned short*) int_ptr = (unsigned short) d;
        break;
    case SGCINTTYPE_UINT:
        *(unsigned int*) int_ptr = (unsigned int) d;
        break;
    case SGCINTTYPE_UINT32:
        *(uint32_t*) int_ptr = (uint32_t) d;
        break;
    case SGCINTTYPE_ULONG:
        *(unsigned long*) int_ptr = (unsigned long) d;
        break;
    case SGCINTTYPE_UINT64:
        *(uint64_t*) int_ptr = (uint64_t) d;
        break;
    }
    return 0;
}

int sg_big_float_set_c_float(sg_big_float_t *dst, const void *float_ptr, enum sg_c_float_type type)
{
    if (!dst || !float_ptr)
        return -1;

    if (type != SGCFLOATTYPE_SFLOAT && type != SGCFLOATTYPE_SDOUBLE)
        return -1;

    double d = (type == SGCFLOATTYPE_SFLOAT) ? (*((float*) float_ptr)) : (*((double*) float_ptr));
    mpf_set_d(dst->mpf, d);
    return 0;
}

int sg_big_float_get_c_float(sg_big_float_t *src, void *float_ptr, enum sg_c_float_type type)
{
    if (!src || !float_ptr)
        return -1;

    if (type != SGCFLOATTYPE_SFLOAT && type != SGCFLOATTYPE_SDOUBLE)
        return -1;

    double d = mpf_get_d(src->mpf);
    (type == SGCFLOATTYPE_SFLOAT) ? (*((float*) float_ptr) = (float) d) : (*((double*) float_ptr) = d);
    return 0;
}

int sg_big_float_set_big_int(sg_big_float_t *dst, sg_big_int_t *src)
{
    if (!dst || !src)
        return -1;

    sg_vsstr_t* str = sg_vsstr_alloc();
    sg_big_int_get_str(src, SGNUMSYS_DEC, str);
    sg_big_float_set_str(dst, sg_vsstr_raw(str), SGNUMSYS_DEC);
    sg_vsstr_free(&str);
    return 0;
}

int sg_big_float_set_big_float(sg_big_float_t *dst, sg_big_float_t *src)
{
    if (!dst || !src)
        return -1;

    mpf_set(dst->mpf, src->mpf);
    return 0;
}

int sg_big_float_set_str(sg_big_float_t *dst, const char *num_str, enum sg_num_sys sys)
{
    int base = SG_COMPUTE_BASE(sys);
    if (!dst || !num_str || base <= 0)
        return -1;

    mpf_set_str(dst->mpf, num_str, base);
    return 0;
}

int sg_big_float_get_str(sg_big_float_t *src, enum sg_num_sys sys, sg_vsstr_t * str)
{
    int base = SG_COMPUTE_BASE(sys);
    if (!src || !str || base <= 0)
        return -1;

    mp_exp_t exp;
    char* s = mpf_get_str(NULL, &exp, base, 0, src->mpf);
    sg_vsstr_cpy(str, s);
    if (exp != 0) {
        long mantissa_length = strlen(s);
        /* rid negative symbol */
        if (mpf_sgn(src->mpf) == -1)
            --mantissa_length;
        sg_vsstr_t* float_exp = sg_vsstr_fmt("e%ld", exp - mantissa_length);
        sg_vsstr_cat(str, sg_vsstr_raw(float_exp));
        sg_vsstr_free(&float_exp);
    }
    free(s);
    return 0;
}

int sg_big_float_get_mantissa_and_exponent(sg_big_float_t *src, enum sg_num_sys sys, sg_vsstr_t * mantissa, size_t* exponent)
{
    int base = SG_COMPUTE_BASE(sys);
    if (!src || !mantissa || base <= 0)
        return -1;

    mp_exp_t exp;
    char* s = mpf_get_str(NULL, &exp, base, 0, src->mpf);
    sg_vsstr_cpy(mantissa, s);
    *exponent = exp;
    free(s);
    return 0;
}

int sg_big_float_add(sg_big_float_t *a, sg_big_float_t *b, sg_big_float_t *res)
{
    if (!a || !b || !res)
        return -1;

    mpf_add(res->mpf, a->mpf, b->mpf);
    return 0;
}

int sg_big_float_sub(sg_big_float_t *a, sg_big_float_t *b, sg_big_float_t *res)
{
    if (!a || !b || !res)
        return -1;

    mpf_sub(res->mpf, a->mpf, b->mpf);
    return 0;
}

int sg_big_float_mul(sg_big_float_t *a, sg_big_float_t *b, sg_big_float_t *res)
{
    if (!a || !b || !res)
        return -1;

    mpf_mul(res->mpf, a->mpf, b->mpf);
    return 0;
}

int sg_big_float_div(sg_big_float_t *a, sg_big_float_t *b, sg_big_float_t *res)
{
    if (!a || !b || !res)
        return -1;

    mpf_div(res->mpf, a->mpf, b->mpf);
    return 0;
}

int sg_big_float_sqrt(sg_big_float_t *a, sg_big_float_t *res)
{
    if (!a || !res)
        return -1;

    mpf_sqrt(res->mpf, a->mpf);
    return 0;
}

enum sg_big_float_cmp_result sg_big_float_cmp(sg_big_float_t *a, sg_big_float_t *b)
{
    int ret = mpf_cmp(a->mpf, b->mpf);
    return ret > 0 ? SGBIGFLOATCMP_A_BIGGER : ret == 0 ? SGBIGFLOATCMP_EQUALS : SGBIGFLOATCMP_B_BIGGER;
}

int sg_big_float_cmp_zero(sg_big_float_t* a)
{
    return mpf_cmp_ui(a->mpf, 0);
}

int sg_big_float_try_to_get_c_int_type(sg_big_float_t *src, enum sg_c_int_type *type)
{
    if (!src || !type)
        return -1;

    if (mpf_fits_sshort_p(src->mpf))
        *type = SGCINTTYPE_SSHORT;
    else if (mpf_fits_ushort_p(src->mpf))
        *type = SGCINTTYPE_USHORT;
    else if (mpf_fits_sint_p(src->mpf))
        *type = SGCINTTYPE_SINT;
    else if (mpf_fits_uint_p(src->mpf))
        *type = SGCINTTYPE_UINT;
    else if (mpf_fits_slong_p(src->mpf))
        *type = SGCINTTYPE_SLONG;
    else if (mpf_fits_ulong_p(src->mpf))
        *type = SGCINTTYPE_ULONG;

    return 0;
}

bool sg_big_float_check_num_str(const char *float_str, enum sg_num_sys sys)
{
    mpf_t z;
    float base = SG_COMPUTE_BASE(sys);

    if (!float_str || !base)
        return false;

    return mpf_init_set_str(z, float_str, base) == 0;
}

int sg_big_float_floor(sg_big_float_t* dst, const sg_big_float_t* src)
{
    if (!dst || !src)
        return -1;
    mpf_floor(dst->mpf, src->mpf);
    return 0;
}

int sg_big_float_ceil(sg_big_float_t* dst, const sg_big_float_t* src)
{
    if (!dst || !src)
        return -1;
    mpf_ceil(dst->mpf, src->mpf);
    return 0;
}

int sg_big_float_trunc(sg_big_float_t* dst, const sg_big_float_t* src)
{
    if (!dst || !src)
        return -1;
    mpf_trunc(dst->mpf, src->mpf);
    return 0;
}