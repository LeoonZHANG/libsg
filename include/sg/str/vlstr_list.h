/*
 * vlstr_list.h
 * Author: wangwei.
 * List to store variable length string.
 */

#ifndef LIBSG_VLSTR_LIST_H
#define LIBSG_VLSTR_LIST_H

#include <stddef.h> /* size_t */
#include "vlstr.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Types. */
typedef struct sg_vlstr_list_real sg_vlstr_list_t;

/* Create a null variable length string list. */
sg_vlstr_list_t *sg_vlstr_list_alloc(void);

/* Get size of variable length string list. */
size_t sg_vlstr_list_size(sg_vlstr_list_t *vl);

/* Get char buffer pointer by zero based index
   from variable length string list. */
char *sg_vlstr_list_get(sg_vlstr_list_t *vl, int index);

/* Add source string to variable length string list. */
int sg_vlstr_list_push(sg_vlstr_list_t *vl, const char *src);

/* Add first num characters of source string to variable length string list. */
int sg_vlstr_list_push2(sg_vlstr_list_t *vl, const char *src, size_t num);

/* Join all sg_vlstr_list_t items to one vlstr. */
sg_vlstr_t *sg_vlstr_list_join(sg_vlstr_list_t *vl);

/* Free variable length string list and all items.
   Attention, vl is a pointer to pointer. */
void sg_vlstr_list_free(sg_vlstr_list_t **vl);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_VLSTR_LIST_H */
