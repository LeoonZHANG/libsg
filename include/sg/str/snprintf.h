/*
 * snprintf.h
 * Author: wangwei.
 * Cross paltform snprintf.
 */

#ifndef LIBSG_SNPRINTF_H
#define LIBSG_SNPRINTF_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(SG_COMPILER_GCC)
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
