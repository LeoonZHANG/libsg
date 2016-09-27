/**
 * vsstr.h
 * Variable-sized string.
 * Thread unsafe.
 */

#ifndef LIBSG_VSSTR_H
#define LIBSG_VSSTR_H

#include <stdarg.h>
/* FIXME: why #include "sg.h" doesn't work */
#include "../sys/type.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Type definition. */
typedef struct vsstr_hd sg_vsstr_t;

/* Create a sg_vsstr_t only the terminator. */
sg_vsstr_t *sg_vsstr_alloc(void);

/* Create a sg_vsstr_t initialized by str. */
sg_vsstr_t *sg_vsstr_alloc2(const char *str);

/* Create a vsstr which open up 'size' bytes memory space. */
sg_vsstr_t *sg_vsstr_alloc3(uint64_t size);

/* Get length of vsstr, its time complexity is O(1), strlen is O(N). */
uint64_t sg_vsstr_len(sg_vsstr_t *str);

/* Get char buffer pointer from vsstr. */
char *sg_vsstr_raw(sg_vsstr_t *str);

/* Copy source to destination.
   Return: destination. */
sg_vsstr_t *sg_vsstr_cpy(sg_vsstr_t *dst, const char *src);

/* Copy at most the first num characters of source to destination.
   Return: destination. */
sg_vsstr_t *sg_vsstr_ncpy(sg_vsstr_t *dst, const char *src, size_t num);

/* Set increment before sg_vsstrcat..., to reduce free / malloc times. */
void sg_vsstr_setinc(sg_vsstr_t *str, uint32_t increment);

/* Join source string to destination string.
   Return: destination. */
sg_vsstr_t *sg_vsstr_cat(sg_vsstr_t *dst, const char *src);

/* Join first num characters of source to destination string.
   Return: destination. */
sg_vsstr_t *sg_vsstr_ncat(sg_vsstr_t *dst, const char *src, size_t num);

/* Create a new sg_vsstr_t which is a duplicate of the string src. */
sg_vsstr_t *sg_vsstr_dup(const char *src);

/* Create a new sg_vsstr_t which is a duplicate of the sg_vsstr_t src. */
sg_vsstr_t *sg_vsstr_dup2(sg_vsstr_t *src);

/* Create a new sg_vsstr_t and clone at most the first num characters.
   of source to the new string. */
sg_vsstr_t *sg_vsstr_ndup(const char *src, size_t num);

/* Create a new sg_vsstr_t and format it. */
#ifdef SG_COMPILER_GCC
sg_vsstr_t *sg_vsstr_fmt(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
#else
sg_vsstr_t *sg_vsstr_fmt(const char *fmt, ...);
#endif

/* Create a new sg_vsstr_t and format it. */
sg_vsstr_t *sg_vsstr_fmt_ap(const char *fmt, va_list ap);

/* Reset data of str to zero, but keep memory spaces of it. */
void sg_vsstr_zero(sg_vsstr_t *str);

/* Free memory of str. */
void sg_vsstr_free(sg_vsstr_t **str);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_VSSTR_H */