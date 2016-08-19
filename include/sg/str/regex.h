/*
 * regex.h
 * Author: wangwei.
 * Regular expression library based on pcre.
 */

#ifndef LIBSG_REGEX_H
#define LIBSG_REGEX_H

#include "../str/vlstr_list.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Types. */
typedef void sg_pattern;

/* Regular expression result callback function type definition.
   data is substring address of re_exec source string.
   size is valid size of substring. */
typedef void (*sg_regex_callback)(const char *data, size_t size, void *context);

/* Compile pattern string into an internal form.
   Return NULL: failure.
   Return valid pointer: succeed. */
sg_pattern *sg_regex_make_pattern(const char *pattern_str);

/* Matches a compiled regular expression against a given subject string.
   Return -1: error.
   Return  0: no match.
   Return >0: match number. */
int sg_regex_exec(char *src, sg_pattern *pat, sg_regex_callback cb, void *context);

/* Matches a compiled regular expression against a given subject string.
   Match results are stored in vlstr_list, remember to free the list. */
sg_vlstr_list *sg_regex_exec2(char *src, sg_pattern *pat);

/* Free internal form of regular expression. */
void sg_regex_free_pattern(sg_pattern **pat);

/* Matches a regular expression string against a given subject string.
   Match results are stored in vlstr_list, remember to free the list. */
sg_vlstr_list *sg_regex_match(char *src, const char *pattern_str);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_REGEX_H */