/**
 * vsbuf.c
 * Variable-sized buffer.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sg/sg.h>
#include <sg/container/vsbuf.h>

struct sg_vsbuf *sg_vsbuf_create(void)
{
    struct sg_vsbuf *buf;

    buf = (struct sg_vsbuf *)malloc(sizeof(struct sg_vsbuf));
    if (!buf) {
        sg_log_err("error");
        return NULL;
    }

    buf->mem = NULL;
    buf->size = 0;
    return buf;
}

int sg_vsbuf_insert(struct sg_vsbuf *buf, const void *mem, size_t size)
{
    size_t new_size;
    void *new_mem;

    SG_ASSERT(buf);
    SG_ASSERT(mem);
    SG_ASSERT(size > 0);

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

void sg_vsbuf_reset(struct sg_vsbuf *buf)
{
    SG_ASSERT(buf);

    if (buf->mem) {
        free(buf->mem);
        buf->mem = NULL;
    }
    buf->size = 0;
}

void sg_vsbuf_destroy(struct sg_vsbuf *buf)
{
    SG_ASSERT(buf);

    if (buf->mem)
        free(buf->mem);
    free(buf);
}