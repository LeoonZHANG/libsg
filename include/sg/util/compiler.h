/**
 * compiler.h
 * Compiler macros.
 */

#ifndef LIBSG_COMPILER_H
#define LIBSG_COMPILER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(__GNUC__)
# define SG_COMPILER_GCC
#elif defined(_MSC_VER)
# define SG_COMPILER_MSVC
#elif defined(__clang__)
# define SG_COMPILER_CLANG
#else
# error Unsupported compiler.
#endif

/* Compiling date time. */
#define SG_COMPILER_DATETIME_STRING (__DATE__ __TIME__)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_COMPILER_H */
