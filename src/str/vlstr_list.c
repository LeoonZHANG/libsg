/*
 * vlstr_list.c
 * Author: wangwei.
 * List to store variable length string.
 */

#include <stdlib.h>
#include <sg/str/vlstr.h>
#include <sg/container/list.h>
#include <sg/util/assert.h>
#include <sg/str/vlstr_list.h>
#include <sg/util/log.h>

struct sg_vlstr_list_real {
    struct sg_list *list;
};

sg_vlstr_list_t *sg_vlstr_list_alloc(void)
{
    sg_vlstr_list_t *vl;

    vl = (sg_vlstr_list_t *)malloc(sizeof(sg_vlstr_list_t));
    if (!vl)
        return NULL;

    vl->list = sg_list_init();
    if (!vl->list) {
        free(vl);
        return NULL;
    }

    return vl;
}

size_t sg_vlstr_list_size(sg_vlstr_list_t *vl)
{
    assert(vl);
    if (!vl->list)
        return 0;

    return (size_t)sg_list_size(vl->list);
}

char *sg_vlstr_list_get(sg_vlstr_list_t *vl, int index)
{
    sg_vlstr *s;

    assert(vl);
    assert(index >= 0);

    s = (sg_vlstr *)sg_list_get_data_ptr(vl->list, (uint32_t)index);
    if (!s)
        return NULL;

    return sg_vlstrraw(s);
}

int sg_vlstr_list_push(sg_vlstr_list_t *vl, const char *src)
{
    sg_vlstr *s;
    struct sg_item *item;

    assert(vl);
    assert(src);

    s = sg_vlstralloc2(src);
    if (!s)
        return -1;

    item = sg_list_add_item(vl->list, ITEMDATATYPE_ATTACH,
                            (void *)s, sizeof(sg_vlstr *), 0, NULL);

    return item ? 0 : -1;
}

int sg_vlstr_list_push2(sg_vlstr_list_t *vl, const char *src, size_t num)
{
    sg_vlstr *s;
    struct sg_item *item;

    assert(vl);
    assert(src);

    s = sg_vlstrndup(src, num);
    if (!s)
        return -1;

    item = sg_list_add_item(vl->list, ITEMDATATYPE_ATTACH, (void *)s, sizeof(sg_vlstr *), 0, NULL);

    return item ? 0 : -1;
}

sg_vlstr *sg_vlstr_list_join(sg_vlstr_list_t *vl)
{
    sg_log_crit("Uncompleted API.");
    abort();
    return NULL;
}

void sg_vlstr_list_free(sg_vlstr_list_t **vl)
{
    struct sg_item *cur;
    sg_vlstr *s;

    assert(vl);
    if (!*vl || !(*vl)->list)
        return;

    cur = sg_list_begin((*vl)->list);
    while (cur) {
        s = (sg_vlstr *)cur->data;
        if (s)
            sg_vlstrfree(&s);

        cur = cur->next;
    }

    sg_list_uninit((*vl)->list);
    free(*vl);
    *vl = NULL;
}