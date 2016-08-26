/*
 * flex_buf.c
 * Author: wangwei.
 * Flexible buffer.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/sg/util/log.h"
#include "../../include/sg/util/assert.h"
#include <sg/container/vlbuf.h>

struct sg_vlbuf *sg_vlbuf_create(void)
{
    struct sg_vlbuf *buf;

    buf = (struct sg_vlbuf *)malloc(sizeof(struct sg_vlbuf));
    if (!buf) {
        sg_log_err("error");
        return NULL;
    }

    buf->mem = NULL;
    buf->size = 0;
    return buf;
}

int sg_vlbuf_insert(struct sg_vlbuf *buf, const void *mem, size_t size)
{
    size_t new_size;
    void *new_mem;

    sg_assert(buf);
    sg_assert(mem);
    sg_assert(size > 0);

    new_size = buf->size + size;

    new_mem = malloc(new_size);
    if (!new_mem) {
        sg_log_err("err");
        return -1;
    }

    if (buf->mem && buf->size > 0)
        memcpy(new_mem, buf->mem, buf->size);
    memcpy((uint8_t *)new_mem + buf->size, mem, size);
    if (buf->mem)
        free(buf->mem);
    buf->mem = new_mem;
    buf->size = new_size;

    return 0;
}

void sg_vlbuf_reset(struct sg_vlbuf *buf)
{
    sg_assert(buf);

    if (buf->mem) {
        free(buf->mem);
        buf->mem = NULL;
    }
    buf->size = 0;
}

void sg_vlbuf_destroy(struct sg_vlbuf *buf)
{
    sg_assert(buf);

    if (buf->mem)
        free(buf->mem);
    free(buf);
}

