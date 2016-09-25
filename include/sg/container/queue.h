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

typedef struct sg_queue sg_queue_t;

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
};

/******************************************
 * queue operation.
 * ***************************************/

/*  Initialise the queue. */
sg_queue_t *sg_queue_alloc(uint64_t capacity);

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

bool sg_queue_try_push(sg_queue_t *self, void *item_val);

bool sg_queue_try_pop(sg_queue_t *self);

void *sg_queue_timeout_pop(sg_queue_t *self, int ms);

void sg_queue_remove_all(sg_queue_t *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_QUEUE_H */
