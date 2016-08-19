/*
 * snprintf.h
 * Author: wangwei.
 * Cross paltform snprintf.
 */

#ifndef LIBSG_SNPRINTF_H
#define LIBSG_SNPRINTF_H

#include "../sys/os.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(__GNUC__)
# include <stdio.h>
# define sg_snprintf snprintf
#else
/* Emulate snprintf() on Windows, _snprintf() doesn't zero-terminate the buffer on overflow... */
int sg_snprintf(char *buf, size_t len, const char *fmt, ...);
#endif


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_SNPRINTF_H */
