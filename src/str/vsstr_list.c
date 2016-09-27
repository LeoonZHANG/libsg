/**
 * vsstr_list.c
 * List to store variablesized string.
 */

#include <stdlib.h>
#include <sg/sg.h>
#include <sg/str/vsstr.h>
#include <sg/container/list.h>
#include <sg/str/vsstr_list.h>


struct sg_vsstr_list_real {
    struct sg_list *list;
};

sg_vsstr_list_t *sg_vsstr_list_alloc(void)
{
    sg_vsstr_list_t *vl;

    vl = (sg_vsstr_list_t *)malloc(sizeof(sg_vsstr_list_t));
    if (!vl)
        return NULL;

    vl->list = sg_list_alloc(NULL);
    if (!vl->list) {
        free(vl);
        return NULL;
    }

    return vl;
}

size_t sg_vsstr_list_size(sg_vsstr_list_t *vl)
{
    SG_ASSERT(vl);
    if (!vl->list)
        return 0;

    return (size_t)vl->list->size;
}
/*
char *sg_vsstr_list_get(sg_vsstr_list_t *vl, int index)
{
    sg_vsstr_t *s;
    int tmp_index = 0;

    SG_ASSERT(vl);
    SG_ASSERT(index >= 0);


    s = (sg_vsstr_t *)sg_list_at(vl->list, (uint64_t)index);
    if (!s)
        return NULL;

    return sg_vsstr_raw(s);
}*/

int sg_vsstr_list_push(sg_vsstr_list_t *vl, const char *src)
{
    sg_vsstr_t *s;
    struct sg_list_item *item;

    SG_ASSERT(vl);
    SG_ASSERT(src);

    s = sg_vsstr_alloc2(src);
    if (!s)
        return -1;

    item = sg_list_add_tail(vl->list, (void *)s);

    return item ? 0 : -1;
}

int sg_vsstr_list_push2(sg_vsstr_list_t *vl, const char *src, size_t num)
{
    sg_vsstr_t *s;
    struct sg_list_item *item;

    SG_ASSERT(vl);
    SG_ASSERT(src);

    s = sg_vsstr_ndup(src, num);
    if (!s)
        return -1;

    item = sg_list_add_tail(vl->list, (void *)s);

    return item ? 0 : -1;
}

sg_vsstr_t *sg_vsstr_list_join(sg_vsstr_list_t *vl)
{
    sg_log_crit("Uncompleted API.");
    abort();
    return NULL;
}

void sg_vsstr_list_free(sg_vsstr_list_t **vl)
{
    struct sg_list_item *cur;
    sg_vsstr_t *s;

    SG_ASSERT(vl);
    if (!*vl || !(*vl)->list)
        return;

    cur = (*vl)->list->first;
    while (cur) {
        s = (sg_vsstr_t *)cur->val;
        if (s)
            sg_vsstr_free(&s);

        cur = cur->next;
    }

    sg_list_free((*vl)->list);
    free(*vl);
    *vl = NULL;
}
