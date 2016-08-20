/*
 * vlstr.h
 * Author: wangwei.
 * Variable length string.
 * Thread unsafe.
 */

#ifndef LIBSG_VLSTR_H
#define LIBSG_VLSTR_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Type definition. */
typedef struct vlstr_hd sg_vlstr;

/* Create a sg_vlstr only the terminator. */
sg_vlstr *sg_vlstralloc(void);

/* Create a sg_vlstr initialized by str. */
sg_vlstr *sg_vlstralloc2(const char *str);

/* Create a vlstr which open up 'size' bytes memory space. */
sg_vlstr *sg_vlstralloc3(uint64_t size);

/* Get length of vlstr, its time complexity is O(1), strlen is O(N). */
uint64_t sg_vlstrlen(sg_vlstr *str);

/* Get char buffer pointer from vlstr. */
char *sg_vlstrraw(sg_vlstr *str);

/* Copy source to destination.
   Return: destination. */
sg_vlstr *sg_vlstrcpy(sg_vlstr *dst, const char *src);

/* Copy at most the first num characters of source to destination.
   Return: destination. */
sg_vlstr *sg_vlstrncpy(sg_vlstr *dst, const char *src, size_t num);

/* Set increment before sg_vlstrcat..., to reduce free / malloc times. */
void sg_vlstrsetinc(sg_vlstr *str, uint32_t increment);

/* Join source string to destination string.
   Return: destination. */
sg_vlstr *sg_vlstrcat(sg_vlstr *dst, const char *src);

/* Join first num characters of source to destination string.
   Return: destination. */
sg_vlstr *sg_vlstrncat(sg_vlstr *dst, const char *src, size_t num);

/* Create a new sg_vlstr which is a duplicate of the string src. */
sg_vlstr *sg_vlstrdup(const char *src);

/* Create a new sg_vlstr which is a duplicate of the sg_vlstr src. */
sg_vlstr *sg_vlstrdup2(sg_vlstr *src);

/* Create a new sg_vlstr and clone at most the first num characters.
   of source to the new string. */
sg_vlstr *sg_vlstrndup(const char *src, size_t num);

/* Create a new sg_vlstr and format it. */
#ifdef __GNUC__
sg_vlstr *sg_vlstrfmt(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
#else
sg_vlstr *sg_vlstrfmt(const char *fmt, ...);
#endif

/* Create a new sg_vlstr and format it. */
sg_vlstr *sg_vlstrfmt_ap(const char *fmt, va_list ap);

/* Reset data of str to zero, but keep memory spaces of it. */
void sg_vlstrempty(sg_vlstr *str);

/* Free memory of str. */
void sg_vlstrfree(sg_vlstr **str);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_VLSTR_H */