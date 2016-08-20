/*
 * number.c
 * Author: wangwei.
 * Integer, float and double number.
 */

#include <float.h>
#include <math.h>
#include "../../include/sg/math/number.h"

enum sg_num_cmp sg_float_cmp(float left, float right)
{
    if (fabs(left - right) < FLT_EPSILON)
        return SGNUMCMP_EQUALS;
    else if (left > right)
        return SGNUMCMP_LEFT_BIGGER;
    else
        return SGNUMCMP_RIGHT_BIGGER;
}

enum sg_num_cmp sg_double_cmp(double left, double right)
{
    if (fabs(left - right) < DBL_EPSILON)
        return SGNUMCMP_EQUALS;
    else if (left > right)
        return SGNUMCMP_LEFT_BIGGER;
    else
        return SGNUMCMP_RIGHT_BIGGER;
}

int sg_float_is_zero(float num)
{
    if (num >= - FLT_EPSILON && num <= FLT_EPSILON)
        return 1;
    else
        return 0;
}

int sg_double_is_zero(double num)
{
    if (num >= - DBL_EPSILON && num <= DBL_EPSILON)
        return 1;
    else
        return 0;
}