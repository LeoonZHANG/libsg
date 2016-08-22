/*
 * number.c
 * Author: wangwei.
 * Integer, float and double number.
 */

#include <float.h>
#include <math.h>
#include "../../include/sg/math/num.h"

enum sg_num_cmp sg_num_cmp_f(float a, float b)
{
    if (fabs(a - b) < FLT_EPSILON)
        return SGNUMCMP_EQUALS;
    else if (a > b)
        return SGNUMCMP_A_BIGGER;
    else
        return SGNUMCMP_B_BIGGER;
}

enum sg_num_cmp sg_num_cmp_d(double a, double b)
{
    if (fabs(a - b) < DBL_EPSILON)
        return SGNUMCMP_EQUALS;
    else if (a > b)
        return SGNUMCMP_A_BIGGER;
    else
        return SGNUMCMP_B_BIGGER;
}

int sg_num_is_zero_f(float num)
{
    if (num >= - FLT_EPSILON && num <= FLT_EPSILON)
        return 1;
    else
        return 0;
}

int sg_num_is_zero_d(double num)
{
    if (num >= - DBL_EPSILON && num <= DBL_EPSILON)
        return 1;
    else
        return 0;
}