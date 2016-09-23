/*
 * string.h
 * Author: wangwei.
 * String process.
 */

#ifndef LIBSG_STRING_H
#define LIBSG_STRING_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Char buffer to lower case. */
int sg_str_to_lower(char *str);

/* Char buffer to upper case. */
int sg_str_to_upper(char *str);

/* Is a string hex number format string. */
bool sg_str_is_hex(const char *src);

/* Is a string decimal number format string. */
bool sg_str_is_decimal(const char *src);

/* Convert a bool style string to bool value. */
int sg_str_to_bool(const char *src, bool *out);

/* Convert a decimal double style string to double value. */
int sg_decimal_str_to_double(const char *src, double *out);

/* Convert a decimal integer style string to integer value. */
int sg_decimal_str_to_int(const char *src, int *out);

/* Delete char from string. */
int sg_str_del_chr(char *str, const char chr);

/* Replace old char with new one. */
int sg_str_rep_chr(char *str, const char old_chr, const char new_chr);

/* Delete sub string from source string. */
int sg_str_del_str(char *str, const char *del);

/* Untest! */
/* Delete string end with first tag. */
/*int sg_str_del_head(char *src, size_t len);*/

/* Untest! */
/* Reverse find sub string from source string. */
char *sg_str_r_str(char *src, char *find);

/* Untest! */
/* Find substring count */
size_t sg_str_sub_count(const char *src, const char *sub);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_STRING_H */
