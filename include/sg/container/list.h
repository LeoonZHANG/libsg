/**
 * list.h
 * Doubly linked list.
 */

#ifndef LIBSG_LIST_H
#define LIBSG_LIST_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_list_item sg_list_item_t;

typedef struct sg_list sg_list_t;

typedef bool (*sg_list_iter_cb_t)(sg_list_item_t *item, void *ctx);

typedef void (*sg_list_free_cb_t)(void *item_val);

/* 请放到c文件中去 */
struct sg_list_item {
    struct sg_list_item *prev;
    struct sg_list_item *next;
    void                *val;
};

/* 请放到c文件中去 */
struct sg_list {
    sg_list_item_t     *first;
    uint64_t           size;
    /* custom free method for sg_list_remove_** | sg_list_free to cleanup items */
    sg_list_free_cb_t  free_cb;
};

enum sg_list_add_position {
    SGLISTADDPOSTION_HEAD = 0,
    SGLISTADDPOSTION_TAIL
};

enum sg_list_add_direction {
    SGLISTADDDIRECTION_LEFT = 0,
    SGLISTADDDIRECTION_RIGHT
};

/******************************************
 * List operation.
 * ***************************************/

sg_list_t *sg_list_alloc(sg_list_free_cb_t free_cb);

uint64_t sg_list_size(sg_list_t *self);

void sg_list_free(sg_list_t *self);


/******************************************
 * item operation.
 * ***************************************/

sg_list_item_t *sg_list_begin(sg_list_t *self);

sg_list_item_t *sg_list_end(sg_list_t *self);

sg_list_item_t *sg_list_item_prev(sg_list_item_t *item);

sg_list_item_t *sg_list_item_next(sg_list_item_t *item);

void sg_list_iter(sg_list_t *self, sg_list_iter_cb_t cb, void *ctx);

void *sg_list_get_item_val(sg_list_item_t *item);

void sg_list_update_item_val(sg_list_item_t *item, void *new_data);

sg_list_item_t *sg_list_add(sg_list_t *self, void *item_val,
                            enum sg_list_add_position);

sg_list_item_t *sg_list_add2(sg_list_t *self, void *item_val,
                             sg_list_item_t *exist_item, enum sg_list_add_direction);

bool sg_list_remove(sg_list_t *self, sg_list_item_t *exist_item);

bool sg_list_remove_all(sg_list_t *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_LIST_H */