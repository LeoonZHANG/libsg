/*
 * pixel.c
 * Author: wangwei.
 * Pixel operations, ported from great ffmpeg.
 */

#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include <stdint-gcc.h>
#include "../../libavutil/frame.h"
#include "../../libavutil/pixdesc.h"
#include "../../libavutil/imgutils.h"
#include "../../libswscale/swscale_internal.h"
#include "../../libswscale/swscale.h"
#include "../../libavcodec/avcodec.h"
#include "../util/log.h"
#include "../util/macro.h"
#include "../util/assert.h"
#include "pixel.h"

#define R 0
#define G 1
#define B 2
#define A 3

#ifdef __GNUC__
#    define px_builtin_constant_p __builtin_constant_p
#else
#    define px_builtin_constant_p(x) 0
#endif

#define PX_CEIL_RSHIFT(a,b) (!px_builtin_constant_p(b) ? -((-(a)) >> (b)) \
                                                       : ((a) + (1<<(b)) - 1) >> (b))

/*
 * divide by 255 and round to nearest
 * apply a fast variant: (X+127)/255 = ((X+127)*257+257)>>16 = ((X+128)*257)>>16
 */
#define FAST_DIV255(x) ((((x) + 128) * 257) >> 16)

/* calculate the unpremultiplied alpha, applying the general equation:
 * alpha = alpha_overlay / ( (alpha_main + alpha_overlay) - (alpha_main * alpha_overlay) )
 * (((x) << 16) - ((x) << 9) + (x)) is a faster version of: 255 * 255 * x
 * ((((x) + (y)) << 8) - ((x) + (y)) - (y) * (x)) is a faster version of: 255 * (x + y)
 */
#define UNPREMULTIPLY_ALPHA(x, y) ((((x) << 16) - ((x) << 9) + (x)) / ((((x) + (y)) << 8) - ((x) + (y)) - (y) * (x)))

static const enum AVPixelFormat alpha_pix_fmts[] = {
        AV_PIX_FMT_YUVA420P, AV_PIX_FMT_YUVA444P,
        AV_PIX_FMT_ARGB, AV_PIX_FMT_ABGR, AV_PIX_FMT_RGBA,
        AV_PIX_FMT_BGRA, AV_PIX_FMT_NONE
};

static const enum { RED = 0, GREEN, BLUE, ALPHA };

struct blend_context{
    int src_w; /* main image width */
    int src_h; /* main image height */
    int dst_w; /* overlay image width */
    int dst_h; /* overlay image height */
    int dst_pix_step[4]; /* steps per pixel for each plane of the main output */
    int src_pix_step[4]; /* steps per pixel for each plane of the overlay */
    int hsub, vsub;      /* chroma subsampling values */
    uint8_t dst_has_alpha;     /* main image has alpha */
    uint8_t dst_is_packed_rgb; /* main image is rgb color space */
    uint8_t dst_rgba_map[4];   /* main image rgba bit map */
    uint8_t src_has_alpha;     /* overlay image has alpha */
    uint8_t src_is_packed_rgb; /* overlay image is rgb color space */
    uint8_t src_rgba_map[4];   /* overlay image rgba bit map */
};

struct conv_context{
    int conv_size;     /* convert size or not */
    int conv_pix_fmt;  /* convert pixel format or not */
    uint32_t dst_w;    /* zoom image width */
    uint32_t dst_h;    /* zoom image height */
    enum AVPixelFormat dst_pix_fmt;
};

int px_fill_rgba_map(uint8_t *rgba_map, enum AVPixelFormat pix_fmt);
int px_fmt_is_in(int fmt, const int *fmts);


int px_fill_rgba_map(uint8_t *rgba_map, enum AVPixelFormat pix_fmt)
{
    switch (pix_fmt) {
        case AV_PIX_FMT_0RGB:
        case AV_PIX_FMT_ARGB:  rgba_map[ALPHA] = 0; rgba_map[RED  ] = 1; rgba_map[GREEN] = 2; rgba_map[BLUE ] = 3; break;
        case AV_PIX_FMT_0BGR:
        case AV_PIX_FMT_ABGR:  rgba_map[ALPHA] = 0; rgba_map[BLUE ] = 1; rgba_map[GREEN] = 2; rgba_map[RED  ] = 3; break;
        case AV_PIX_FMT_RGB48LE:
        case AV_PIX_FMT_RGB48BE:
        case AV_PIX_FMT_RGBA64BE:
        case AV_PIX_FMT_RGBA64LE:
        case AV_PIX_FMT_RGB0:
        case AV_PIX_FMT_RGBA:
        case AV_PIX_FMT_RGB24: rgba_map[RED  ] = 0; rgba_map[GREEN] = 1; rgba_map[BLUE ] = 2; rgba_map[ALPHA] = 3; break;
        case AV_PIX_FMT_BGR48LE:
        case AV_PIX_FMT_BGR48BE:
        case AV_PIX_FMT_BGRA64BE:
        case AV_PIX_FMT_BGRA64LE:
        case AV_PIX_FMT_BGRA:
        case AV_PIX_FMT_BGR0:
        case AV_PIX_FMT_BGR24: rgba_map[BLUE ] = 0; rgba_map[GREEN] = 1; rgba_map[RED  ] = 2; rgba_map[ALPHA] = 3; break;
        case AV_PIX_FMT_GBRAP:
        case AV_PIX_FMT_GBRP:  rgba_map[GREEN] = 0; rgba_map[BLUE ] = 1; rgba_map[RED  ] = 2; rgba_map[ALPHA] = 3; break;
        default:                    /* unsupported */
            return AVERROR(EINVAL);
    }
    return 0;
}

int px_fmt_is_in(int fmt, const int *fmts)
{
    const int *p;

    for (p = fmts; *p != -1; p++) {
        if (fmt == *p)
            return 1;
    }
    return 0;
}

struct blend_context *pixel_blend_get_context(void)
{
    struct blend_context *ctx;

    ctx = (struct blend_context *)malloc(sizeof(struct blend_context));
    if(!ctx){
        sg_log_err("Blend context malloc failure.");
        return NULL;
    }

    return ctx;
}

int pixel_blend_set_main(struct blend_context *ctx, enum AVPixelFormat fmt,
                         int width, int height)
{
    const AVPixFmtDescriptor *pix_desc;

    if(ctx == NULL)
        return -1;
    if(fmt <= AV_PIX_FMT_NONE)
        return -1;

    pix_desc = av_pix_fmt_desc_get(fmt);
    av_image_fill_max_pixsteps(ctx->dst_pix_step, NULL, pix_desc);
    ctx->hsub = pix_desc->log2_chroma_w;
    ctx->vsub = pix_desc->log2_chroma_h;
    ctx->dst_is_packed_rgb =
            px_fill_rgba_map(ctx->dst_rgba_map, fmt) >= 0 ? (uint8_t)1 : (uint8_t)0;
    ctx->dst_has_alpha = (uint8_t)px_fmt_is_in(fmt, (const int*)alpha_pix_fmts);
    ctx->dst_w = width;
    ctx->dst_h = height;

    return 0;
}

int pixel_blend_set_overlay(struct blend_context *ctx, enum AVPixelFormat fmt,
                            int width, int height)
{
    const AVPixFmtDescriptor *pix_desc;

    if(ctx == NULL)
        return -1;
    if(fmt <= AV_PIX_FMT_NONE)
        return -1;

    pix_desc = av_pix_fmt_desc_get(fmt);
    av_image_fill_max_pixsteps(ctx->src_pix_step, NULL, pix_desc);
    ctx->src_is_packed_rgb =
            px_fill_rgba_map(ctx->src_rgba_map, fmt) >= 0 ? (uint8_t)1 : (uint8_t)0;
    ctx->src_has_alpha = (uint8_t)px_fmt_is_in(fmt, (const int*)alpha_pix_fmts);
    ctx->src_w = width;
    ctx->src_h = height;

    return 0;
}

int pixel_blend(struct blend_context *s,
                uint8_t *dst[8], int dst_linesize[8],
                const uint8_t *src[8], const int src_linesize[8],
                int x, int y)
{
    int i, imax, j, jmax, k, kmax;
    const int dst_w = s->dst_w;
    const int dst_h = s->dst_h;
    const int src_w = s->src_w;
    const int src_h = s->src_h;

    if(s == NULL)
        return -1;
    if(dst == NULL)
        return -1;
    if(src == NULL)
        return -1;
    if (x >= dst_w || x+src_w < 0 ||
        y >= dst_h || y+src_h < 0)
        return -1; /* no intersection */

    if (s->dst_is_packed_rgb) {
        uint8_t alpha;          ///< the amount of overlay to blend on to main
        const int dr = s->dst_rgba_map[R];
        const int dg = s->dst_rgba_map[G];
        const int db = s->dst_rgba_map[B];
        const int da = s->dst_rgba_map[A];
        const int dstep = s->dst_pix_step[0];
        const int sr = s->src_rgba_map[R];
        const int sg = s->src_rgba_map[G];
        const int sb = s->src_rgba_map[B];
        const int sa = s->src_rgba_map[A];
        const int sstep = s->src_pix_step[0];
        const int dst_has_alpha = s->dst_has_alpha;
        uint8_t *s, *sp, *d, *dp;

        i = MAX(-y, 0);
        sp = src[0] + i     * src_linesize[0];
        dp = dst[0] + (y+i) * dst_linesize[0];

        for (imax = MIN(-y + dst_h, src_h); i < imax; i++) {
            j = MAX(-x, 0);
            s = sp + j     * sstep;
            d = dp + (x+j) * dstep;

            for (jmax = MIN(-x + dst_w, src_w); j < jmax; j++) {
                alpha = s[sa];

                // if the main channel has an alpha channel, alpha has to be calculated
                // to create an un-premultiplied (straight) alpha value
                if (dst_has_alpha && alpha != 0 && alpha != 255) {
                    uint8_t alpha_d = d[da];
                    alpha = UNPREMULTIPLY_ALPHA(alpha, alpha_d);
                }

                switch (alpha) {
                    case 0:
                        break;
                    case 255:
                        d[dr] = s[sr];
                        d[dg] = s[sg];
                        d[db] = s[sb];
                        break;
                    default:
                        // main_value = main_value * (1 - alpha) + overlay_value * alpha
                        // since alpha is in the range 0-255, the result must divided by 255
                        d[dr] = FAST_DIV255(d[dr] * (255 - alpha) + s[sr] * alpha);
                        d[dg] = FAST_DIV255(d[dg] * (255 - alpha) + s[sg] * alpha);
                        d[db] = FAST_DIV255(d[db] * (255 - alpha) + s[sb] * alpha);
                }
                if (dst_has_alpha) {
                    switch (alpha) {
                        case 0:
                            break;
                        case 255:
                            d[da] = s[sa];
                            break;
                        default:
                            // apply alpha compositing: main_alpha += (1-main_alpha) * overlay_alpha
                            d[da] += FAST_DIV255((255 - d[da]) * s[sa]);
                    }
                }
                d += dstep;
                s += sstep;
            }
            dp += dst_linesize[0];
            sp += src_linesize[0];
        }
    } else {
        const int dst_has_alpha = s->dst_has_alpha;
        if (dst_has_alpha) {
            uint8_t alpha;          ///< the amount of overlay to blend on to main
            uint8_t *s, *sa, *d, *da;

            i = MAX(-y, 0);
            sa = src[3] + i     * src_linesize[3];
            da = dst[3] + (y+i) * dst_linesize[3];

            for (imax = MIN(-y + dst_h, src_h); i < imax; i++) {
                j = MAX(-x, 0);
                s = sa + j;
                d = da + x+j;

                for (jmax = MIN(-x + dst_w, src_w); j < jmax; j++) {
                    alpha = *s;
                    if (alpha != 0 && alpha != 255) {
                        uint8_t alpha_d = *d;
                        alpha = UNPREMULTIPLY_ALPHA(alpha, alpha_d);
                    }
                    switch (alpha) {
                        case 0:
                            break;
                        case 255:
                            *d = *s;
                            break;
                        default:
                            // apply alpha compositing: main_alpha += (1-main_alpha) * overlay_alpha
                            *d += FAST_DIV255((255 - *d) * *s);
                    }
                    d += 1;
                    s += 1;
                }
                da += dst_linesize[3];
                sa += src_linesize[3];
            }
        }

        for (i = 0; i < 3; i++) {
            int hsub = i ? s->hsub : 0;
            int vsub = i ? s->vsub : 0;
            int src_wp = PX_CEIL_RSHIFT(src_w, hsub);
            int src_hp = PX_CEIL_RSHIFT(src_h, vsub);
            int dst_wp = PX_CEIL_RSHIFT(dst_w, hsub);
            int dst_hp = PX_CEIL_RSHIFT(dst_h, vsub);
            int yp = y>>vsub;
            int xp = x>>hsub;
            uint8_t *s, *sp, *d, *dp, *a, *ap;

            j = MAX(-yp, 0);
            sp = src[i] + j         * src_linesize[i];
            dp = dst[i] + (yp+j)    * dst_linesize[i];
            ap = src[3] + (j<<vsub) * src_linesize[3];

            for (jmax = MIN(-yp + dst_hp, src_hp); j < jmax; j++) {
                k = MAX(-xp, 0);
                d = dp + xp+k;
                s = sp + k;
                a = ap + (k<<hsub);

                for (kmax = MIN(-xp + dst_wp, src_wp); k < kmax; k++) {
                    int alpha_v, alpha_h, alpha;

                    // average alpha for color components, improve quality
                    if (hsub && vsub && j+1 < src_hp && k+1 < src_wp) {
                        alpha = (a[0] + a[src_linesize[3]] +
                                 a[1] + a[src_linesize[3]+1]) >> 2;
                    } else if (hsub || vsub) {
                        alpha_h = hsub && k+1 < src_wp ?
                                  (a[0] + a[1]) >> 1 : a[0];
                        alpha_v = vsub && j+1 < src_hp ?
                                  (a[0] + a[src_linesize[3]]) >> 1 : a[0];
                        alpha = (alpha_v + alpha_h) >> 1;
                    } else
                        alpha = a[0];

                    // if the main channel has an alpha channel, alpha has to be calculated
                    // to create an un-premultiplied (straight) alpha value
                    if (dst_has_alpha && alpha != 0 && alpha != 255) {
                        // average alpha for color components, improve quality
                        uint8_t alpha_d;
                        if (hsub && vsub && j+1 < src_hp && k+1 < src_wp) {
                            alpha_d = (d[0] + d[src_linesize[3]] +
                                       d[1] + d[src_linesize[3]+1]) >> 2;
                        } else if (hsub || vsub) {
                            alpha_h = hsub && k+1 < src_wp ?
                                      (d[0] + d[1]) >> 1 : d[0];
                            alpha_v = vsub && j+1 < src_hp ?
                                      (d[0] + d[src_linesize[3]]) >> 1 : d[0];
                            alpha_d = (alpha_v + alpha_h) >> 1;
                        } else
                            alpha_d = d[0];
                        alpha = UNPREMULTIPLY_ALPHA(alpha, alpha_d);
                    }

                    switch (alpha) {
                        case 0:
                            break;
                        case 255:
                            *d = *s;
                            break;
                        default:
                            *d = FAST_DIV255(*d * (255 - alpha) + *s * alpha);
                    }

                    s++;
                    d++;
                    a += 1 << hsub;
                }
                dp += dst_linesize[i];
                sp += src_linesize[i];
                ap += (1 << vsub) * src_linesize[3];
            }
        }
    }

    return 0;
}

void pixel_blend_free_context(struct blend_context **ctx)
{
    assert(ctx);

    if(*ctx){
        free(*ctx);
        *ctx = NULL;
    }
}

AVFrame *pixel_alloc_frm(const uint32_t w, const uint32_t h,
                         enum AVPixelFormat fmt)
{
    int size;
    AVFrame *frm_head;        /* Frame head: AVFrame. */
    uint8_t *frm_body = NULL; /* Frame data. */

    assert(w > 4);
    assert(h > 4);
    assert(fmt > AV_PIX_FMT_NONE);

    frm_head = av_frame_alloc();
    if(!frm_head) {
        sg_log_err("av_frame_alloc failure.");
        goto failed;
    }

    size = avpicture_get_size(fmt, w, h);
    if(size <= 0) {
        sg_log_err("avpicture_get_size(%d, %d, %d) %d.", fmt, w, h, size);
        goto failed;
    }

    frm_body = malloc((size_t)size);
    if(!frm_body) {
        sg_log_err("Frame memory malloc failure.");
        goto failed;
    }

    /* Bind data buffer to frame head. */
    if(avpicture_fill((AVPicture *)frm_head, frm_body, fmt, w, h) <= 0) {
        sg_log_err("Frame avpicture_fill failure.");
        goto failed;
    }

    frm_head->width  = w;
    frm_head->height = h;
    frm_head->format = fmt;
    frm_head->pts    = 0;

    return frm_head;

failed:
    av_frame_free(&frm_head);
    if(frm_body)
        free(frm_body);
    return NULL;
}

void pixel_free_frm(AVFrame **src)
{
    uint8_t *frm_body;

    assert(src);
    if(!*src)
        return;

    frm_body = (*src)->data[0];
    av_frame_free(src);
    if(frm_body)
        free(frm_body);
}

struct conv_context *pixel_conv_get_context(void)
{
    struct conv_context *ctx;

    ctx = (struct conv_context *)malloc(sizeof(struct conv_context));
    if(!ctx) {
        sg_log_err("Conversion context malloc failure.");
        return NULL;
    }
    memset(ctx, 0, sizeof(struct conv_context));
    ctx->dst_pix_fmt = AV_PIX_FMT_NONE;

    return ctx;
}

int pixel_conv_set_dst_fmt(struct conv_context *ctx, enum AVPixelFormat fmt)
{
    assert(ctx);
    assert(fmt > AV_PIX_FMT_NONE && fmt < AV_PIX_FMT_NB);

    ctx->conv_pix_fmt = 1;
    ctx->dst_pix_fmt  = fmt;

    return 0;
}

int pixel_conv_set_dst_size(struct conv_context *ctx, uint32_t w, uint32_t h)
{
    assert(ctx);
    assert(w > 4);
    assert(h > 4);

    ctx->conv_size = 1;
    ctx->dst_w     = w;
    ctx->dst_h     = h;

    return 0;
}

AVFrame *pixel_conv_alloc(struct conv_context *ctx, const AVFrame *src)
{
    struct SwsContext *sctx;
    AVFrame *frm;
    uint32_t dst_w;
    uint32_t dst_h;
    enum AVPixelFormat dst_fmt;

    assert(ctx);
    if(ctx->conv_pix_fmt == 1) {
        assert(ctx->dst_pix_fmt > AV_PIX_FMT_NONE);
        assert(ctx->dst_pix_fmt < AV_PIX_FMT_NB);
    }
    if(ctx->conv_size == 1) {
        assert(ctx->dst_w > 4);
        assert(ctx->dst_h > 4);
    }
    assert(src);
    assert(src->format > AV_PIX_FMT_NONE && src->format < AV_PIX_FMT_NB);
    assert(src->data);
    assert(src->data[0]);

    dst_w   = (ctx->conv_size == 1) ? ctx->dst_w : (uint32_t)src->width;
    dst_h   = (ctx->conv_size == 1) ? ctx->dst_h : (uint32_t)src->height;
    dst_fmt = (ctx->conv_pix_fmt == 1) ? ctx->dst_pix_fmt
                                       : (enum AVPixelFormat)src->format;

    frm = pixel_alloc_frm(dst_w, dst_h, dst_fmt);

    sctx = sws_getContext(src->width, src->height,
                          (enum AVPixelFormat)src->format,
                          dst_w, dst_h, dst_fmt,
                          SWS_BILINEAR, NULL, NULL, NULL);
    if(!sctx) {
        sg_log_err("sws_getContext failure.");
        return NULL;
    }

    sws_scale(sctx, src->data, src->linesize, 0,
              src->height /* Not dst height. */, frm->data, frm->linesize);

    sws_freeContext(sctx);
    return frm;
}

void pixel_conv_free_context(struct conv_context **ctx)
{
    assert(ctx);

    if(*ctx) {
        free(*ctx);
        *ctx = NULL;
    }
}

AVFrame *pixel_resize_frm(const AVFrame *src, uint32_t new_w, uint32_t new_h)
{
    AVFrame *frm;
    struct conv_context *cctx;

    assert(src);
    assert(src->data);
    assert(src->data[0]);
    assert(new_w > 4);
    assert(new_h > 4);

    cctx = pixel_conv_get_context();
    if(!cctx)
        return NULL;
    pixel_conv_set_dst_size(cctx, new_w, new_h);
    frm = pixel_conv_alloc(cctx, src);

    pixel_conv_free_context(&cctx);
    return frm;
}