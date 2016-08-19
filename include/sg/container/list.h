/*
 * list.h
 * Author: wangwei.
 * Doubly linked list, extended with primary key and seek.
 */

#ifndef LIBSG_LIST_H
#define LIBSG_LIST_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum item_data_type {
    ITEMDATATYPE_NONE   = 0,
    ITEMDATATYPE_ATTACH = 1,
    ITEMDATATYPE_REOPEN = 2,
};

struct sg_item {
    char                id[40]; /* Set item key for easy lookup. */
    void                *data;
    uint32_t            data_size;
    int                 data2;
    enum item_data_type data_type;
    struct sg_item      *prev;
    struct sg_item      *next;
};

struct sg_list {
    struct sg_item *first;
};


/******************************************
 * Item operation.
 * ***************************************/
/* Initialize a list item. At this point it is not part of any list. */
struct sg_item *sg_item_init(enum item_data_type data_type,
                             void *data, size_t data_size,
                             int data2, const char *id);

/* Returns iterator to the next item. */
struct sg_item *sg_item_next(struct sg_item *item);

/* Clone sg_item from 'src'. The item->data memory
   will be re-opened and copied, even if src is attached. */
struct sg_item *sg_item_clone(struct sg_item *src);

/* Copy item data to user buffer 'out'.
   out_size must equals item->data_size. */
int sg_item_copy_data(struct sg_item *item, void *out,
                      int out_size);

/* Update item data pointer if item is MIDT_ATTACH type.
   Update item data memory if item is MIDT_REOPEN type,
   release original memory if new data_size if different. */
int sg_item_update_data(struct sg_item *item, void *data,
                        size_t data_size);

/* Free item data, keep item. */
void sg_item_free_data(struct sg_item *item);

/* Free item data and item. */
void sg_item_free(struct sg_item *item);


/******************************************
 * List and item operation.
 * ***************************************/
/* Returns iterator to the first item in the list. */
struct sg_item *sg_list_begin(struct sg_list *list);

/* Returns iterator to the last item in the list. */
struct sg_item *sg_list_end(struct sg_list *list);

/* Returns item by it's index in the list. */
struct sg_item *sg_list_get_item_ptr(struct sg_list *list, uint32_t idx);

/* Returns data pointer by item index in the list. */
void *sg_list_get_data_ptr(struct sg_list *list, uint32_t idx);

/* Search first item by string id. */
struct sg_item *sg_list_seek_id(struct sg_list *list, const char *id);

/* Search first item by string id and data2. */
struct sg_item *sg_list_seek2(struct sg_list *list, const char *id,
                              int data2);

/* Returns true is the item is part of a list, false otherwise. */
bool sg_item_is_in_list(struct sg_list *list, struct sg_item *item);

/* Attach the 'item' to the list tail. Priot to
   insertion item should not be part of any list.
   It only add "sg_item *" to "list", will not re-open
   memory space for item. */
int sg_list_attach_tail(struct sg_list *list, struct sg_item *item);

/* Create new item and attach it to the list tail. */
struct sg_item *sg_list_add_item(struct sg_list *list,
                                 enum item_data_type data_type,
                                 void *data,
                                 size_t data_size,
                                 int data2,
                                 const char *id);

/* Removes the item from the list.
   Returns 0 is succeed, negative error number otherwise. */
int sg_list_erase_item(struct sg_list *list, struct sg_item *item);

/* Removes the items by given data2 from the list.
   Returns 0 is succeed, negative error number otherwise. */
int sg_list_erase_items_by_data2(struct sg_list *list, int data2);

/* Removes all the items from the list.
   Returns 0 is succeed, negative error number otherwise. */
int sg_list_erase_all(struct sg_list *list);


/******************************************
 * List operation.
 * ***************************************/
/*  Initialise the list. */
struct sg_list *sg_list_init(void);

/* Returns list size. */
int sg_list_size(struct sg_list *list);

/* Returns item count of given data2. */
int sg_list_size_by_data2(struct sg_list *list, int data2);

/* If the list is empty, returns ture. Otherwise, returns false. */
bool sg_list_is_empty(struct sg_list *list);

/* Clone 'src' list, it will re-open and copy item->data memory.
   Even if source items are attached. */
struct sg_list *sg_list_clone(struct sg_list *src);

/* Untested! */
/* Serialize the list. */
int sg_list_serialize(struct sg_list *src, uint8_t **out_buf,
                      size_t *out_size);

/* Untested! */
/* Deserialize to a new list. */
struct sg_list *sg_list_deserialize(uint8_t *mem, size_t mem_size);

/* Cleanup list. */
void sg_list_uninit(struct sg_list *list);

#define SAFE_MW_LIST_UNINIT(P) if((P) != NULL) { sg_list_uninit(P); (P) = NULL; }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_LIST_H */