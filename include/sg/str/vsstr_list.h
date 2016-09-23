/*
 * vsstr_list.h
 * Author: wangwei.
 * List to store variable-sized string.
 */

#ifndef LIBSG_VSSTR_LIST_H
#define LIBSG_VSSTR_LIST_H

#include <sg/sg.h>
#include "vsstr.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Types. */
typedef struct sg_vsstr_list_real sg_vsstr_list_t;

/* Create a null variable length string list. */
sg_vsstr_list_t *sg_vsstr_list_alloc(void);

/* Get size of variable length string list. */
size_t sg_vsstr_list_size(sg_vsstr_list_t *vl);

/* Get char buffer pointer by zero based index
   from variable length string list. */
//char *sg_vsstr_list_get(sg_vsstr_list_t *vl, int index);

/* Add source string to variable length string list. */
int sg_vsstr_list_push(sg_vsstr_list_t *vl, const char *src);

/* Add first num characters of source string to variable length string list. */
int sg_vsstr_list_push2(sg_vsstr_list_t *vl, const char *src, size_t num);

/* Join all sg_vsstr_list_t items to one vsstr. */
sg_vsstr_t *sg_vsstr_list_join(sg_vsstr_list_t *vl);

/* Free variable length string list and all items.
   Attention, vl is a pointer to pointer. */
void sg_vsstr_list_free(sg_vsstr_list_t **vl);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_VSSTR_LIST_H */