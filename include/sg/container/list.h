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

typedef bool (*sg_list_match_cb_t)(void *item_val_a, void *item_val_b);

typedef void (*sg_list_free_cb_t)(void *data);

struct sg_list_item {
    struct sg_list_item *prev;
    struct sg_list_item *next;
    void                *val;
};

struct sg_list {
    sg_list_item_t     *first;
    uint64_t           size;
    /* custom match method for sg_list_find to check item data equals or not */
    sg_list_match_cb_t match_cb;
    /* custom free method for sg_list_remove_** | sg_list_free to cleanup items */
    sg_list_free_cb_t  free_cb;
};


/******************************************
 * List operation.
 * ***************************************/

sg_list_t *sg_list_alloc(sg_list_free_cb_t free_cb, sg_list_match_cb_t match_cb);

sg_list_t *sg_list_clone(sg_list_t *self);

void sg_list_free(sg_list_t *self);


/******************************************
 * item operation.
 * ***************************************/

bool sg_list_item_is_in_list(sg_list_t *self, sg_list_item_t *item);

sg_list_item_t *sg_list_find(sg_list_t *self, void *item_val);

sg_list_item_t *sg_list_add_left(sg_list_t *self, void *item_val, sg_list_item_t *exist_item);

sg_list_item_t *sg_list_add_right(sg_list_t *self, void *item_val, sg_list_item_t *exist_item);

sg_list_item_t *sg_list_add_head(sg_list_t *self, void *item_val);

sg_list_item_t *sg_list_add_tail(sg_list_t *self, void *item_val);

bool sg_list_remove(sg_list_t *self, sg_list_item_t *exist_item);

bool sg_list_remove_all(sg_list_t *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_LIST_H */