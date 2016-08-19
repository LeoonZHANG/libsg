/*
 * flag.c
 * Author: wangwei.
 * Run flag for thread.
 */

#include <stdlib.h>
#include <sg/sys/flag.h>
#include <sg/sys/mutex.h>
#include <sg/util/assert.h>

#define FLAG_INIT_VALUE 0x0

struct flag_body {
    int             value;
    struct sg_mutex mutex;
};


sg_flag_t *sg_flag_create(void)
{
    struct flag_body *f;

    f = (struct flag_body *)malloc(sizeof(struct flag_body));
    if (!f)
        return NULL;

    f->value = FLAG_INIT_VALUE;
    sg_mutex_create(&(f->mutex));

    return f;
}

int sg_flag_read(sg_flag_t *f)
{
    int value;

    assert(f);

    sg_mutex_lock(&(f->mutex));
    value = f->value;
    sg_mutex_unlock(&(f->mutex));

    return value;
}

void sg_flag_write(sg_flag_t *f, int value)
{
    assert(f);

    sg_mutex_lock(&(f->mutex));
    f->value = value;
    sg_mutex_unlock(&(f->mutex));
}

void sg_flag_destroy(sg_flag_t **f)
{
    assert(f);
    if (!(*f))
        return;

    sg_mutex_destroy(&((*f)->mutex));
    free(*f);
    *f = NULL;
}