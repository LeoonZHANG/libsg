/*
 * sleep.h
 * Author: wangwei.
 * Unified interface to sleep in milliseconds.
 */

#ifndef LIBSG_SLEEP_H
#define LIBSG_SLEEP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Platform independent implementation of sleep in millionsecond. */
void sg_sleep(uint32_t ms);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_SLEEP_H */
