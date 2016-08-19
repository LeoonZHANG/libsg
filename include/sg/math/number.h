/*
 * number.h
 * Author: wangwei.
 * Integer, float and double number.
 */

/*http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
http://blog.chinaunix.net/uid-20279362-id-3848329.html*/

#ifndef LIBSG_NUMBER_H
#define LIBSG_NUMBER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* number comparison results */
enum sg_num_cmp {
    SGNUMCMP_LEFT_BIGGER  = 0, /* Left number is bigger than right. */
    SGNUMCMP_EQUALS       = 1, /* Left number equals right. */
    SGNUMCMP_RIGHT_BIGGER = 2  /* Right number is bigger than left. */
};

enum sg_num_cmp sg_float_cmp(float left, float right);

enum sg_num_cmp sg_double_cmp(double left, double right);

int sg_float_is_zero(float num);

int sg_double_is_zero(double num);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_NUMBER_H */