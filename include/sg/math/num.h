/*
 * num.h
 * Author: wangwei.
 * Common integer, float and double number.
 */

/*http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
http://blog.chinaunix.net/uid-20279362-id-3848329.html*/

#ifndef LIBSG_NUM_H
#define LIBSG_NUM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* number comparison results */
enum sg_num_cmp {
    SGNUMCMP_A_BIGGER = 0, /* Left number is bigger than right. */
    SGNUMCMP_EQUALS   = 1, /* Left number equals right. */
    SGNUMCMP_B_BIGGER = 2  /* Right number is bigger than left. */
};

enum sg_num_cmp sg_num_cmp_f(float a, float b);

enum sg_num_cmp sg_num_cmp_d(double a, double b);

int sg_num_is_zero_f(float num);

int sg_num_is_zero_d(double num);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_NUM_H */