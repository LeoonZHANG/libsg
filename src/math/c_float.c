/*
 * c_float.c
 * Author: wangwei.
 * Extended API for regular floating-pointer number in C.
 */

#include <float.h>
#include <math.h>
#include "../../include/sg/math/c_float.h"

enum sg_c_float_cmp sg_c_float_cmp_f(float a, float b)
{
    if (fabs(a - b) < FLT_EPSILON)
        return SGCFLOATCMP_EQUALS;
    else if (a > b)
        return SGCFLOATCMP_A_BIGGER;
    else
        return SGCFLOATCMP_B_BIGGER;
}

enum sg_c_float_cmp sg_c_float_cmp_d(double a, double b)
{
    if (fabs(a - b) < DBL_EPSILON)
        return SGCFLOATCMP_EQUALS;
    else if (a > b)
        return SGCFLOATCMP_A_BIGGER;
    else
        return SGCFLOATCMP_B_BIGGER;
}

int sg_c_float_is_zero_f(float num)
{
    if (num >= - FLT_EPSILON && num <= FLT_EPSILON)
        return 1;
    else
        return 0;
}

int sg_c_float_is_zero_d(double num)
{
    if (num >= - DBL_EPSILON && num <= DBL_EPSILON)
        return 1;
    else
        return 0;
}