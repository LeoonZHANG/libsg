/*
 * list.c
 * Author: wangwei.
 * Doubly linked list, extended with primary key and seek.
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sg/util/log.h>
#include <sg/container/list.h>
#include <sg/util/assert.h>

/* Item head for serialize / deserialize. */
struct sg_item_hdr {
    uint32_t data_size;
    int32_t  data2;
    char     id[40];
};


/******************************************
 * Item operation.
 * ***************************************/

struct sg_item *sg_item_init(enum item_data_type data_type,
                             void *data, size_t data_size,
                             int data2, const char *id)
{
    struct sg_item *item;
    void *src_data;

    assert(data_type != ITEMDATATYPE_NONE);
    assert(data);
    assert(data_size > 0);
    if (id)
        assert(strlen(id) < 40);

    item = (struct sg_item *)calloc(1, sizeof(struct sg_item));
    if (!item) {
        sg_log_err("sg_item_init failure, alloc sg_item returns NULL.");
        return NULL;
    }

    if (data_type == ITEMDATATYPE_REOPEN) {
        src_data = calloc(1, data_size);
        if (!src_data) {
            sg_log_err("sg_item_init failure, alloc %d returns NULL.", data_size);
            free(item);
            item = NULL;
            return NULL;
        }
        memcpy(src_data, data, data_size);
    } else
        src_data = data;

    item->prev = NULL;
    item->next = NULL;
    item->data = src_data;
    item->data_size = data_size;
    item->data_type = data_type;
    item->data2 = data2;
    if (id)
        snprintf(item->id, 40, "%s", id);

    return item;
}

struct sg_item *sg_item_next(struct sg_item *item)
{
    assert(item);

    return item->next;
}

struct sg_item *sg_item_clone(struct sg_item *src)
{
    assert(src);

    /* If data is a pointer, and use ITEMDATATYPE_REOPEN,
     * the data will become pointer to original pointer after clone. */
    return sg_item_init(src->data_type/*ITEMDATATYPE_REOPEN*/, src->data,
                        src->data_size, src->data2, src->id);
}

int sg_item_copy_data(struct sg_item *item, void *out,
                       int out_size)
{
    assert(item);
    assert(out);
    assert(out_size > 0);

    if (item->data && item->data_size == 0)
        return -1;
    if (out_size != item->data_size)
        return -1;

    memcpy(out, item->data, out_size);

    return 0;
}

int sg_item_update_data(struct sg_item *item, void *data,
                         size_t data_size)
{
    int ret = 0;

    assert(item);
    assert(data);
    assert(data_size > 0);

    if (item->data_type == ITEMDATATYPE_ATTACH) {
        item->data = data;
        item->data_size = data_size;
    } else {
        if (item->data_size == data_size)
            memcpy(item->data, data, data_size);
        else {
            sg_item_free_data(item);
            item->data = malloc(data_size);
            if (!item->data) {
                ret = -1;
                item->data_size = 0;
            }
            memcpy(item->data, data, data_size);
            item->data_size = data_size;
        }
    }

    return ret;
}

void sg_item_free_data(struct sg_item *item)
{
    assert(item);

    if ((item->data_type == ITEMDATATYPE_REOPEN) && item->data) {
        free(item->data);
        item->data = NULL;
    }
}

void sg_item_free(struct sg_item *item)
{
    assert(item);

    sg_item_free_data(item);
    free(item);
}


/******************************************
 * List and item operation.
 * ***************************************/

struct sg_item *sg_list_begin(struct sg_list *list)
{
    assert(list);

    return list->first;
}

struct sg_item *sg_list_end(struct sg_list *list)
{
    struct sg_item *cur;

    assert(list);

    cur = sg_list_begin(list);
    while (cur) {
        if (!(cur->next))
            break;
        cur = cur->next;
    }

    return cur;
}

struct sg_item *sg_list_get_item_ptr(struct sg_list *list, uint32_t item_idx)
{
    int i = 0;
    struct sg_item *cur;
    struct sg_item *res = NULL;

    assert(list);

    while (1) {
        if (i == 0)
            cur = sg_list_begin(list);
        else
            cur = cur->next;

        if (!cur)
            break;

        if (i == item_idx) {
            res = cur;
            break;
        }
        i++;
    }

    return res;
}

void *sg_list_get_data_ptr(struct sg_list *list, uint32_t idx)
{
    struct sg_item *item;
    void *data = NULL;

    assert(list);

    item = sg_list_get_item_ptr(list, idx);
    if (item)
        data = item->data;

    return data;
}

struct sg_item *sg_list_seek_id(struct sg_list *list, const char *id)
{
    struct sg_item *cur;
    struct sg_item *res = NULL;

    assert(list);
    assert(id);
    assert(strlen(id) > 0);

    cur = sg_list_begin(list);
    while (cur) {
        if (strlen(cur->id) > 0 && strcmp(cur->id, id) == 0) {
            res = cur;
            break;
        }
        cur = cur->next;
    }

    return res;
}

struct sg_item *sg_list_seek2(struct sg_list *list, const char *id, int data2)
{
    struct sg_item *cur;
    struct sg_item *res = NULL;

    assert(list);
    assert(id);
    assert(strlen(id) > 0);

    cur = sg_list_begin(list);
    while (cur) {
        if (cur->data2 == data2
           && strlen(cur->id) > 0
           && strcmp(cur->id, id) == 0) {
            res = cur;
            break;
        }
        cur = cur->next;
    }

    return res;
}

bool sg_item_is_in_list(struct sg_list *list, struct sg_item *item)
{
    int i = 0;
    struct sg_item *cur;

    assert(list);
    assert(item);

    do {
        if (i == 0)
            cur = sg_list_begin(list);
        else
            cur = cur->next;

        if (cur == item)
            return true;
        i++;
    } while (cur);
    return false;
}

int sg_list_attach_tail(struct sg_list *list, struct sg_item *item)
{
    struct sg_item *end;

    assert(list);
    assert(item);

    /* Check id/data2 conflict before attaching. */
    if (strlen(item->id) > 0) {
        if (sg_list_seek2(list, item->id, item->data2)) {
            sg_log_err("Item with id %s & data2 %d has been found.", item->id, item->data2);
            return -1;
        }
    }

    end = sg_list_end(list);
    if (end) {
        end->next = item;
        item->prev = end;
        item->next = NULL;
    } else {
        list->first = item;
        item->prev = NULL;
        item->next = NULL;
    }

    return 0;
}

struct sg_item *sg_list_add_item(struct sg_list *list,
                                 enum item_data_type data_type,
                                 void *data,
                                 size_t data_size,
                                 int data2,
                                 const char *id)
{
    struct sg_item *item;

    item = sg_item_init(data_type, data, data_size, data2, id);
    if (!item)
        return NULL;

    if (sg_list_attach_tail(list, item) == 0)
        return item;

    return NULL;
}

int sg_list_erase_item(struct sg_list *list, struct sg_item *item)
{
    struct sg_item *prev;
    struct sg_item *next;
    struct sg_item *first;

    assert(list);
    assert(item);

    if (!sg_item_is_in_list(list, item)) {
        sg_log_err("Cannot erase item from list which is not in it.");
        return -1;
    }

    prev = item->prev;
    next = item->next;
    first = sg_list_begin(list);

    if (first == item)
        list->first = next;

    if (prev)
        prev->next = next;

    if (next)
        next->prev = prev;

    sg_item_free(item);

    return 0;
}

int sg_list_erase_items_by_data2(struct sg_list *list, int data2)
{
    int i = 0;
    struct sg_item *cur, *data2_item;
    struct sg_list *data2_list;

    assert(list);

    data2_list = sg_list_init();
    assert(data2_list);

    do {
        if (i == 0)
            cur = sg_list_begin(list);
        else
            cur = cur->next;

        if (cur && cur->data2 == data2) {
            data2_item = sg_item_init(ITEMDATATYPE_ATTACH, cur,
                                      sizeof(struct sg_item *), 0, NULL);
            sg_list_attach_tail(data2_list, data2_item);
        }
        i++;
    } while (cur);

    i = 0;
    do {
        if (i == 0)
            cur = sg_list_begin(data2_list);
        else
            cur = cur->next;

        if (cur)
            sg_list_erase_item(list, (struct sg_item *)cur->data);
        i++;
    } while (cur);

    sg_list_uninit(data2_list);

    return 0;
}

int sg_list_erase_all(struct sg_list *list)
{
    struct sg_item *item;

    assert(list);

    do {
        item = sg_list_begin(list);
        if (item)
            sg_list_erase_item(list, item);
    } while (item);

    return 0;
}


/******************************************
 * List operation.
 * ***************************************/

struct sg_list *sg_list_init(void)
{
    struct sg_list *list;

    list = (struct sg_list *)malloc(sizeof(struct sg_list));
    if (list)
        list->first = NULL;
    else
        sg_log_err("sg_list malloc failure.");

    return list;
}

int sg_list_size(struct sg_list *list)
{
    int size = 0;
    struct sg_item *cur;

    assert(list);

    cur = sg_list_begin(list);
    while (cur) {
        size ++;
        cur = cur->next;
    }

    return size;
}

int sg_list_size_by_data2(struct sg_list *list, int data2)
{
    int size = 0;
    struct sg_item *cur;

    assert(list);

    cur = sg_list_begin(list);
    while (cur) {
        if (cur->data2 == data2)
            size ++;
        cur = cur->next;
    }

    return size;
}

bool sg_list_is_empty(struct sg_list *list)
{
    assert(list);

    return (list->first ? false : true);
}

struct sg_list *sg_list_clone(struct sg_list *src)
{
    struct sg_item *cur, *clone;
    struct sg_list *clone_list;

    assert(src);

    clone_list = sg_list_init();
    if (!clone_list)
        return NULL;

    cur = sg_list_begin(src);
    while (cur) {
        clone = sg_item_clone(cur);
        if (clone)
            sg_list_attach_tail(clone_list, clone);
        cur = cur->next;
    }

    return clone_list;
}

int sg_list_serialize(struct sg_list *src, uint8_t **out_mem,
                      size_t *out_size)
{
    int i, list_size;
    struct sg_item *item;
    void *mem;
    size_t swap = 0;

    assert(src);
    assert(out_mem);
    assert(out_size);

    *out_mem = NULL;
    *out_size = 0;

    list_size = sg_list_size(src);
    if (list_size == 0)
        return 0;

    *out_size += list_size * sizeof(struct sg_item_hdr);
    item = sg_list_begin(src);
    while (item) {
        *out_size += item->data_size;
        item = item->next;
    }

    *out_mem = (uint8_t *)malloc(*out_size);
    if (!*out_mem) {
        sg_log_err("malloc failure.");
        return -1;
    }

    for(i = 0; i < list_size; i++) {
        item = sg_list_get_item_ptr(src, i);
        mem = &(*out_mem)[swap];
        snprintf(((struct sg_item_hdr *)mem)->id, 40, "%s", item->id);
        ((struct sg_item_hdr *)mem)->data_size = item->data_size;
        ((struct sg_item_hdr *)mem)->data2 = item->data2;
        swap += sizeof(struct sg_item_hdr);
        mem = &(*out_mem)[swap];
        memcpy(mem, item->data, item->data_size);
        swap += item->data_size;
    }

    return 0;
}

/* Deserialize to a new list. */
struct sg_list *sg_list_deserialize(uint8_t *mem, size_t size)
{
    uint8_t *mem_cur = mem;
    struct sg_item *item;
    struct sg_list *res;

    res = sg_list_init();
    if (!mem || size == 0)
        return res;

    while (mem_cur < mem + size) {
        item = sg_item_init(ITEMDATATYPE_REOPEN,
                            mem_cur + sizeof(struct sg_item_hdr),
                            ((struct sg_item_hdr *)mem_cur)->data_size,
                            ((struct sg_item_hdr *)mem_cur)->data2,
                            ((struct sg_item_hdr *)mem_cur)->id);
        sg_list_attach_tail(res, item);
        mem_cur += sizeof(struct sg_item_hdr) +
                ((struct sg_item_hdr *)mem_cur)->data_size;
    }

    return res;
}

void sg_list_uninit(struct sg_list *list)
{
    assert(list);

    sg_list_erase_all(list);
    free(list);

    return;
}