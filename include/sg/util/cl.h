/**
 * cl.h
 * Author: wangwei.
 * Compiler macros.
 */

#ifndef LIBSG_CL_H
#define LIBSG_CL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(__GNUC__)
# define SG_CL_GCC
#elif defined(_MSC_VER)
# define SG_CL_MSC
#else
# error Unsupported compiler.
#endif

/* Compiling date time. */
#define SG_CL_DATETIME_STRING (__DATE__ ## __TIME__)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_CL_H */
