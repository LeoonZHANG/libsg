/**
 * queue.h
 * blocking and non-blocking queue.
 *
 */

#ifndef LIBSG_QUEUE_H
#define LIBSG_QUEUE_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef struct sg_queue_item sg_queue_item_t;

typedef struct sg_queue sg_queue_t;

typedef bool (*sg_queue_match_cb_t)(void *item_val_a, void *item_val_b);

typedef void (*sg_queue_free_cb_t)(void *item_val);


struct sg_queue_item {
    sg_queue_item_t *next;
    void            *val;
};

struct sg_queue {
    sg_queue_item_t     *head;
    sg_queue_item_t     *tail;
    uint64_t            size;
    sg_queue_match_cb_t match_cb;
    sg_queue_free_cb_t  free_cb; /* call this function when remove_item */
};

/******************************************
 * queue operation.
 * ***************************************/

/*  Initialise the queue. */
sg_queue_t *sg_queue_alloc(sg_queue_match_cb_t match_cb, sg_queue_free_cb_t free_cb);

sg_queue_t *sg_queue_clone(sg_queue_t *self);

void sg_queue_free(sg_queue_t *self);


/******************************************
 * item operation.
 * ***************************************/

/*  Returns 1 if item is a part of a queue. 0 otherwise. */
bool sg_queue_item_is_in_queue(sg_queue_t *queue, sg_queue_item_t *item);

sg_queue_item_t *sg_queue_find(sg_queue_t *self, void *item_val);

/*  Inserts one element into the queue. */
void sg_queue_push(sg_queue_t *self, void *item_val);

/*  Retrieves one element from the queue. The element is removed
    from the queue. Returns NULL if the queue is empty. */
sg_queue_item *sg_queue_pop(sg_queue_t *self);

/*  Remove the item and free it with free_cb registered in queue. */
void sg_queue_remove(sg_queue_t *self, sg_queue_item_t *item);

void sg_queue_remove_all(sg_queue_t *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_QUEUE_H */
