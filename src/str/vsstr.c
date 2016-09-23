/**
 * vsstr.c
 * Variable-sized string.
 * Thread unsafe.
 */

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sg/sg.h>
#include <sg/str/vsstr.h>
#include <sg/util/trick.h>


/* vsstr head */
struct vsstr_hd {
    char     *buf;      /* char buffer pointer */
    uint64_t open_size; /* open memory size */
    uint64_t used_size; /* used memory size */
    uint32_t increment; /* increment when sg_vsstr_cat... */
};

sg_vsstr_t *sg_vsstr_alloc(void)
{
    return sg_vsstr_alloc3(1);
}

sg_vsstr_t *sg_vsstr_alloc2(const char *str)
{
    sg_vsstr_t *v;

    SG_ASSERT(str);

    v = sg_vsstr_alloc3(strlen(str) + 1);
    if (!v)
        return NULL;

    return sg_vsstr_cpy(v, str);
}

sg_vsstr_t *sg_vsstr_alloc3(uint64_t size)
{
    struct vsstr_hd *v;

    v = (struct vsstr_hd *)malloc(sizeof(struct vsstr_hd));
    if (!v)
        sg_log_err("Vlstr init failure.");
    memset(v, 0, sizeof(struct vsstr_hd));

    v->open_size = size > 0 ? size : 1 /* terminator */;
    v->buf = (char *)malloc(v->open_size);
    if (!v->buf) {
        sg_vsstr_free(&v);
        return NULL;
    }
    memset(v->buf, 0, v->open_size);

    return v;
}

uint64_t sg_vsstr_len(sg_vsstr_t *str)
{
    SG_ASSERT(str);

    return str->used_size - 1 /* terminator */;
}

char *sg_vsstr_raw(sg_vsstr_t *str)
{
    return str ? str->buf : NULL;
}

sg_vsstr_t *sg_vsstr_cpy(sg_vsstr_t *dst, const char *src)
{
    SG_ASSERT(dst);
    SG_ASSERT(src);

    if (strlen(src) == 0) {
        dst->used_size = 0;
        memset(dst->buf, 0, dst->open_size);
        return dst;
    }

    if (dst->open_size <= strlen(src) + 1) {
        if (dst->buf) {
            free(dst->buf);
            dst->buf = NULL;
        }
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

sg_vsstr_t *sg_vsstr_ncpy(sg_vsstr_t *dst, const char *src, size_t num)
{
    SG_ASSERT(src);
    if (num == 0)
        return dst;

    sg_vsstr_zero(dst);
    return sg_vsstr_ncat(dst, src, num);
}

void sg_vsstr_setinc(sg_vsstr_t *str, uint32_t increment)
{
    SG_ASSERT(str);

    str->increment = increment;
}

sg_vsstr_t *sg_vsstr_cat(sg_vsstr_t *dst, const char *src)
{
    SG_ASSERT(dst);
    SG_ASSERT(src);

    return sg_vsstr_ncat(dst, src, strlen(src));
}

sg_vsstr_t *sg_vsstr_ncat(sg_vsstr_t *dst, const char *src, size_t num)
{
    char *new_buf;
    uint32_t inc;     /* increment */
    uint64_t new_len; /* without terminator */
    uint64_t cat_len; /* without terminator */

    SG_ASSERT(dst);
    SG_ASSERT(src);

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
            sg_vsstr_free(&dst);
            sg_log_err("Vlstr new buffer alloc failure.");
            return NULL;
        }
        strcpy(new_buf, dst->buf);
        strncpy(&new_buf[strlen(new_buf)], src, cat_len);
        new_buf[new_len] = 0;
        dst->open_size += inc;
        if (dst->buf) {
            free(dst->buf);
            dst->buf = NULL;
        }
        dst->buf = new_buf;
    } else {
        strncpy(&dst->buf[strlen(dst->buf)], src, cat_len);
        *(dst->buf + new_len) = 0;
    }
    dst->used_size = new_len + 1;

    return dst;
}

sg_vsstr_t *sg_vsstr_dup(const char *src)
{
    SG_ASSERT(src);

    return sg_vsstr_ndup(src, strlen(src) + 1);
}

sg_vsstr_t *sg_vsstr_dup2(sg_vsstr_t *src)
{
    SG_ASSERT(src);
    SG_ASSERT(src->buf);

    return sg_vsstr_ndup(src->buf, strlen(src->buf) + 1);
}

sg_vsstr_t *sg_vsstr_ndup(const char *src, size_t num)
{
    sg_vsstr_t *v;

    SG_ASSERT(src);

    v = sg_vsstr_alloc();
    if (!v)
        return NULL;

    return sg_vsstr_ncat(v, src, num);
}

sg_vsstr_t *sg_vsstr_fmt(const char *fmt, ...)
{
    sg_vsstr_t *v = NULL;
    va_list ap;

    SG_ASSERT(fmt);

    va_start(ap, fmt);
    v = sg_vsstr_fmt_ap(fmt, ap);
    va_end(ap);

    return v;
}

sg_vsstr_t *sg_vsstr_fmt_ap(const char *fmt, va_list ap)
{
    sg_vsstr_t *v = NULL;
    int ret;
    int increment = 256;
    size_t open_size = 0;
    char *buf = NULL;

    SG_ASSERT(fmt);
    SG_ASSERT(ap);

    //printf("vsstr fmt ap->fmt:%s\n", fmt);
    //printf("vsstr fmt ap->ap:%s\n", va_arg(ap, char *));

#if 0//HAVE_C99_VSNPRINTF
    len = vsnprintf(NULL, 0, fmt, ap);
    v = sg_vsstralloc3(len);
    if (!v)
        return NULL;

    vsnprintf(v->buf, len, fmt, ap);
#else
    while (1) {
        if (buf) {
            free(buf);
            buf = NULL;
        }
        /* FIXME: Invalid bounds checking! */
        if (open_size + increment > SG_LIMIT_SIZE_T_MAX) {
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
            if (buf) {
                free(buf);
                buf = NULL;
            }
            sg_log_err("vlsprintf snprintf failure.");
            return NULL;
        }
        if (ret + 1 /* terminator */ < open_size) {
            v = sg_vsstr_alloc();
            if (v->buf) {
                free(v->buf);
                v->buf = NULL;
            }
            v->buf = buf;
            v->open_size = strlen(buf) + 1;
            v->used_size = v->open_size;
            break;
        }
    }
#endif

    return v;
}

void sg_vsstr_zero(sg_vsstr_t *str)
{
    SG_ASSERT(str);

    memset(str->buf, 0, str->open_size);
    str->used_size = 0;
}

void sg_vsstr_free(sg_vsstr_t **str)
{
    SG_ASSERT(str);
    if (!*str)
        return;

    if ((*str)->buf)
        free((*str)->buf);
    free(*str);
    *str = NULL;
}