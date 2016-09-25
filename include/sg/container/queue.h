/**
 * queue.h
 * non-blocking queue.
 * 自己实现,不要基于开源代码
 */

#ifndef LIBSG_QUEUE_H
#define LIBSG_QUEUE_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_queue_item sg_queue_item_t;

typedef struct sg_queue sg_queue_t;

typedef bool (*sg_queue_iter_cb_t)(sg_queue_item_t *item, void *ctx);

typedef void (*sg_queue_free_cb_t)(void *item_val);

/* 放进queue.c */
struct sg_queue_item {
    sg_queue_item_t *next;
    void            *val;
};

/* 放进queue.c */
struct sg_queue {
    sg_queue_item_t     *head;
    sg_queue_item_t     *tail;
    uint64_t            size;
    sg_queue_free_cb_t  free_cb;
};

/******************************************
 * queue operation.
 * ***************************************/

/*  Initialise the queue, 如果参数传0表示无限大小. */
sg_queue_t *sg_queue_alloc(uint64_t capacity, sg_queue_free_cb_t cb);

uint64_t sg_queue_size(sg_queue_t *self);

void sg_queue_free(sg_queue_t *self);


/******************************************
 * item operation.
 * ***************************************/

/*  Inserts one element into the queue. */
bool sg_queue_push(sg_queue_t *self, void *item_val);

/*  Retrieves one element from the queue. The element is removed
    from the queue. Returns NULL if the queue is empty. */
void *sg_queue_pop(sg_queue_t *self);

void sg_queue_iter(sg_queue_t *self, sg_queue_iter_cb_t cb, void *ctx);

void *sg_queue_get_item_val(sg_queue_item_t *item);

void sg_queue_update_item_val(sg_queue_t *item, void *val);

void sg_queue_remove_item(sg_queue_t *self, sg_queue_item_t *item);

void sg_queue_remove_all(sg_queue_t *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_QUEUE_H */
