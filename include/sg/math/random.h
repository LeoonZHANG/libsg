/*
 * random.h
 * Author: wangwei.
 * "True" random number generator.
 */

#ifndef LIBSG_RANDOM_H
#define LIBSG_RANDOM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

long sg_rondom_l(void);

float sg_rondom_f(void);

long sg_rondom_range_l(long min, long max);

float sg_rondom_range_f(float min, float max);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_RANDOM_H */
