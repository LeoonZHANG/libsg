/*
 * flag.c
 * Author: wangwei.
 * Run flag for thread.
 */

#include <stdlib.h>
#include <sg/sg.h>
#include <sg/sys/flag.h>
#include <sg/sys/mutex.h>

#define FLAG_INIT_VALUE 0x0

struct sg_flag_real {
    int             value;
    sg_mutex_t      *mutex;
};


sg_flag_t *sg_flag_create(void)
{
    struct sg_flag_real *f;

    f = (struct sg_flag_real *)malloc(sizeof(struct sg_flag_real));
    if (!f)
        return NULL;

    f->value = FLAG_INIT_VALUE;
    f->mutex = sg_mutex_create();

    return (sg_flag_t *)f;
}

int sg_flag_read(sg_flag_t *f)
{
    int value;

    assert(f);

    sg_mutex_lock(((struct sg_flag_real *)f)->mutex);
    value = ((struct sg_flag_real *)f)->value;
    sg_mutex_unlock(((struct sg_flag_real *)f)->mutex);

    return value;
}

void sg_flag_write(sg_flag_t *f, int value)
{
    assert(f);

    sg_mutex_lock(((struct sg_flag_real *)f)->mutex);
    ((struct sg_flag_real *)f)->value = value;
    sg_mutex_unlock(((struct sg_flag_real *)f)->mutex);
}

void sg_flag_destroy(sg_flag_t *f)
{
    assert(f);

    sg_mutex_destroy(((struct sg_flag_real *)f)->mutex);
    free(f);
}