/*
 * md5.c
 * Author: wangwei.
 * Md5 hash tool based on the work of Alexander Peslyak and WaterJuice.
 */

#include <stdlib.h>
#include <string.h>
#include "../../3rdparty/CryptLib/lib/LibMd5.h"
#include "../../include/compress/md5.h"
#include "../../include/util/def.h"
#include "../../include/util/assert.h"


void md5_hash_fmt(MD5_HASH *raw, struct sg_md5_hash *out, enum sg_md5_fmt fmt);

struct sg_md5_hash sg_md5_easy_buf(void *buf, size_t len, enum sg_md5_fmt fmt)
{
    Md5Context          ctx;
    MD5_HASH            hash;
    struct sg_md5_hash  res;

    assert(buf);
    assert(len > 0);

    ZERO(hash, sizeof(struct sg_md5_hash));

    Md5Initialise(&ctx);
    Md5Update(&ctx, buf, (uint32_t)len);
    Md5Finalise(&ctx, &hash);

    md5_hash_fmt(&hash, &res, fmt);

    return res;
}

struct sg_md5_hash sg_md5_easy_str(const char *str, enum sg_md5_fmt fmt)
{
    assert(str);
    assert(strlen(str) > 0);

    return sg_md5_easy_buf((void *)str, strlen(str), fmt);
}

struct sg_md5_hash sg_md5_easy_file(const char *path, enum sg_md5_fmt fmt)
{
    sg_md5_ctx          *ctx;
    uint8_t             buf[1024];
    FILE                *fp;
    size_t              size;
    struct sg_md5_hash  nil;

    assert(path);
    assert(strlen(path) > 0);

    ZERO(nil, sizeof(struct sg_md5_hash));

    ctx = sg_md5_start();
    if (!ctx)
        return nil;

    fp = fopen(path, "rb");
    if (!fp)
        return nil;

    while (!feof(fp)) {
        size = fread(buf, 1, 1024, fp);

        if (size > 0)
            sg_md5_update(ctx, buf, size);
    }

    return sg_md5_finish(ctx, fmt);
}

sg_md5_ctx *sg_md5_start(void)
{
    Md5Context *ctx;

    ctx = (Md5Context *)malloc(sizeof(Md5Context));
    if (!ctx)
        return NULL;

    Md5Initialise(ctx);

    return ctx;
}

void sg_md5_update(sg_md5_ctx *mc, void *buf, size_t len)
{
    Md5Context *ctx = (Md5Context *)mc;

    assert(ctx);
    assert(buf);
    assert(len > 0);

    Md5Update(ctx, buf, (uint32_t)len);
}

struct sg_md5_hash sg_md5_finish(sg_md5_ctx *mc, enum sg_md5_fmt fmt)
{
    Md5Context          *ctx = (Md5Context *)mc;
    MD5_HASH            hash;
    struct sg_md5_hash  res;

    assert(ctx);

    Md5Finalise(ctx, &hash);
    md5_hash_fmt(&hash, &res, fmt);

    return res;
}

/* Copy or convert raw md5 hash bytes by output format. */
void md5_hash_fmt(MD5_HASH *raw, struct sg_md5_hash *out, enum sg_md5_fmt fmt)
{
    int i;

    assert(raw);
    assert(out);
    assert(fmt >= SGMD5FMT_RAW && fmt <= SGMD5FMT_STR);

    memset(out, 0, 33);

    if (fmt == SGMD5FMT_RAW)
        memcpy(out->buf, raw->bytes, 16);
    else
        for(i = 0; i < MD5_HASH_SIZE; i++)
            sprintf(&((char *)out->buf)[i * 2], "%02x", raw->bytes[i] );
}
