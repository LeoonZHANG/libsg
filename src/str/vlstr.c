/*
 * vlstr.c
 * Author: wangwei.
 * Variable length string.
 * Thread unsafe.
 */

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sg/str/vlstr.h>
#include <sg/util/log.h>
#include <sg/util/def.h>
#include <sg/util/bound.h>
#include <sg/util/assert.h>

/* vlstr head */
struct vlstr_hd {
    char     *buf;      /* char buffer pointer */
    uint64_t open_size; /* open memory size */
    uint64_t used_size; /* used memory size */
    uint32_t increment; /* increment when strcat... */
};

sg_vlstr_t *sg_vlstralloc(void)
{
    return sg_vlstralloc3(1);
}

sg_vlstr_t *sg_vlstralloc2(const char *str)
{
    sg_vlstr_t *v;

    assert(str);

    v = sg_vlstralloc3(strlen(str) + 1);
    if (!v)
        return NULL;

    return sg_vlstrcpy(v, str);
}

sg_vlstr_t *sg_vlstralloc3(uint64_t size)
{
    struct vlstr_hd *v;

    v = (struct vlstr_hd *)malloc(sizeof(struct vlstr_hd));
    if (!v)
        sg_log_err("Vlstr init failure.");
    memset(v, 0, sizeof(struct vlstr_hd));

    v->open_size = size > 0 ? size : 1 /* terminator */;
    v->buf = (char *)malloc(v->open_size);
    if (!v->buf) {
        sg_vlstrfree(&v);
        return NULL;
    }
    memset(v->buf, 0, v->open_size);

    return v;
}

uint64_t sg_vlstrlen(sg_vlstr_t *str)
{
    assert(str);

    return str->used_size - 1 /* terminator */;
}

char *sg_vlstrraw(sg_vlstr_t *str)
{
    return str ? str->buf : NULL;
}

sg_vlstr_t *sg_vlstrcpy(sg_vlstr_t *dst, const char *src)
{
    assert(dst);
    assert(src);

    if (strlen(src) == 0) {
        dst->used_size = 0;
        memset(dst->buf, 0, dst->open_size);
        return dst;
    }

    if (dst->open_size <= strlen(src) + 1) {
        SAFE_FREE(dst->buf);
        dst->buf = (char *)malloc(strlen(src) + 1);
        dst->open_size = strlen(src) + 1;
        dst->used_size = dst->open_size;
        strcpy(dst->buf, src);
        return dst;
    }

    dst->used_size = strlen(src) + 1;
    strcpy(dst->buf, src);
    return dst;
}

sg_vlstr_t *sg_vlstrncpy(sg_vlstr_t *dst, const char *src, size_t num)
{
    assert(src);
    if (num == 0)
        return dst;

    sg_vlstrempty(dst);
    return sg_vlstrncat(dst, src, num);
}

void sg_vlstrsetinc(sg_vlstr_t *str, uint32_t increment)
{
    assert(str);

    str->increment = increment;
}

sg_vlstr_t *sg_vlstrcat(sg_vlstr_t *dst, const char *src)
{
    assert(dst);
    assert(src);

    return sg_vlstrncat(dst, src, strlen(src));
}

sg_vlstr_t *sg_vlstrncat(sg_vlstr_t *dst, const char *src, size_t num)
{
    char *new_buf;
    uint32_t inc;     /* increment */
    uint64_t new_len; /* without terminator */
    uint64_t cat_len; /* without terminator */

    assert(dst);
    assert(src);

    if (strlen(src) == 0 || num == 0)
        return dst;

    cat_len = strlen(src) <= num ? strlen(src) : num;
    new_len = strlen(dst->buf) + cat_len;
    if (dst->open_size - dst->used_size > cat_len)
        inc = 0;
    else
        inc = (dst->increment > cat_len) ? dst->increment : (uint32_t)cat_len;

    if (inc > 0) {
        new_buf = (char *)malloc(dst->open_size + inc);
        if (!new_buf) {
            sg_vlstrfree(&dst);
            sg_log_err("Vlstr new buffer alloc failure.");
            return NULL;
        }
        strcpy(new_buf, dst->buf);
        strncpy(&new_buf[strlen(new_buf)], src, cat_len);
        new_buf[new_len] = 0;
        dst->open_size += inc;
        SAFE_FREE(dst->buf);
        dst->buf = new_buf;
    } else {
        strncpy(&dst->buf[strlen(dst->buf)], src, cat_len);
        *(dst->buf + new_len) = 0;
    }
    dst->used_size = new_len + 1;

    return dst;
}

sg_vlstr_t *sg_vlstrdup(const char *src)
{
    assert(src);

    return sg_vlstrndup(src, strlen(src) + 1);
}

sg_vlstr_t *sg_vlstrdup2(sg_vlstr_t *src)
{
    assert(src);
    assert(src->buf);

    return sg_vlstrndup(src->buf, strlen(src->buf) + 1);
}

sg_vlstr_t *sg_vlstrndup(const char *src, size_t num)
{
    sg_vlstr_t *v;

    assert(src);

    v = sg_vlstralloc();
    if (!v)
        return NULL;

    return sg_vlstrncat(v, src, num);
}

sg_vlstr_t *sg_vlstrfmt(const char *fmt, ...)
{
    sg_vlstr_t *v = NULL;
    va_list ap;

    assert(fmt);

    va_start(ap, fmt);
    v = sg_vlstrfmt_ap(fmt, ap);
    va_end(ap);

    return v;
}

sg_vlstr_t *sg_vlstrfmt_ap(const char *fmt, va_list ap)
{
    sg_vlstr_t *v = NULL;
    int ret;
    int increment = 256;
    size_t open_size = 0;
    char *buf = NULL;

    assert(fmt);
    assert(ap);

    //printf("vlstr fmt ap->fmt:%s\n", fmt);
    //printf("vlstr fmt ap->ap:%s\n", va_arg(ap, char *));

#if 0//HAVE_C99_VSNPRINTF
    len = vsnprintf(NULL, 0, fmt, ap);
    v = sg_vlstralloc3(len);
    if (!v)
        return NULL;

    vsnprintf(v->buf, len, fmt, ap);
#else
    while (1) {
        SAFE_FREE(buf);
        /* Invalid bounds checking! */
        if (open_size + increment > BD_SIZE_T_MAX) {
            sg_log_err("vlsprintf malloc size is too big.");
            return NULL;
        }
        open_size += increment;
        buf = (char *)malloc(open_size);
        if (!buf) {
            sg_log_err("vlsprintf malloc failure.");
            return NULL;
        }
        ret = vsnprintf(buf, open_size, fmt, ap);
        if (ret < 0) {
            SAFE_FREE(buf);
            sg_log_err("vlsprintf snprintf failure.");
            return NULL;
        }
        if (ret + 1 /* terminator */ < open_size) {
            v = sg_vlstralloc();
            SAFE_FREE(v->buf);
            v->buf = buf;
            v->open_size = strlen(buf) + 1;
            v->used_size = v->open_size;
            break;
        }
    }
#endif

    return v;
}

void sg_vlstrempty(sg_vlstr_t *str)
{
    assert(str);

    memset(str->buf, 0, str->open_size);
    str->used_size = 0;
}

void sg_vlstrfree(sg_vlstr_t **str)
{
    assert(str);
    if (!*str)
        return;

    if ((*str)->buf)
        free((*str)->buf);
    free(*str);
    *str = NULL;
}
