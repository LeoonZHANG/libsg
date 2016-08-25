/*
 * image.c
 * Author: wangwei.
 * Image process library.
 */

#include "SDL2/SDL_surface.h"
#include "SDL2/SDL_pixels.h"
#include "../util/log.h"
#include "../util/assert.h"
#include "../util/dlib.h"
#include "../util/module.h"
#include "image_sdl.h"
#include "pixel.h"
#include "image.h"

SDL_Surface *cv_mat_to_sdl_surface(image_cv *src);
image_cv *sdl_surface_to_cv_mat(SDL_Surface *src);
int image_draw_text_in(const char *text, enum text_encoding enc,
                       struct rgba8888 color, int use_padding_bottom,
                       int32_t x, int32_t y, int32_t padding_bottom,
                       const char *font_name, uint32_t font_size,
                       int font_style, struct image *dst);
/* Update new image_cv to struct image members, and free old instance. */
int image_flush_instance(struct image *dst, image_cv *instance);


/* Image APIs function types from OpenCV. */
typedef image_cv *(*image_cv_load_t)(const char *path);
typedef image_cv *(*image_cv_create_t)(uint32_t w, uint32_t h,
                                       struct rgba8888 color);
typedef image_cv *(*image_cv_create2_t)(uint32_t w, uint32_t h, int depth_bytes, int ch);
typedef image_cv *(*image_cv_create_from_data_t)(uint32_t w, uint32_t h,
                                                 uint8_t depth_bytes, uint8_t ch,
                                                 uint8_t *data, uint64_t size,
                                                 enum pix_fmt pix_fmt);
typedef image_cv *(*image_cv_clone_t)(image_cv *src);
typedef size_t (*image_cv_width_t)(image_cv *img);
typedef size_t (*image_cv_height_t)(image_cv *img);
typedef int (*image_cv_depth_bytes_t)(image_cv *img);
typedef int (*image_cv_channels_t)(image_cv *img);
typedef enum pix_fmt (*image_cv_format_t)(image_cv *img);
typedef uint8_t *(*image_cv_data_t)(image_cv *img);
typedef size_t (*image_cv_data_size_t)(image_cv *img);
typedef image_cv *(*image_cv_resize_t)(image_cv *src, uint32_t new_w, uint32_t new_h);
typedef image_cv *(*image_cv_add_alpha_channel_t)(image_cv *src);
typedef image_cv *(*image_cv_2d_rotate_t)(image_cv *src, float angle);
typedef image_cv *(*image_cv_3d_rotate_t)(image_cv *src, float angle);
typedef image_cv *(*image_cv_trim_t)(image_cv *src, uint32_t left,
                                     uint32_t top, uint32_t right,
                                     uint32_t bottom);
typedef image_cv *(*image_cv_set_alpha_t)(image_cv *src, float alpha);
typedef int (*image_cv_erase_t)(image_cv *src);
typedef void (*image_cv_show_t)(const char *window_title, image_cv *img);
typedef int (*image_cv_save_t)(image_cv *src, const char *path);
typedef void (*image_cv_free_t)(image_cv **img);

/* Image APIs from OpenCV. */
static image_cv_load_t image_cv_load;
static image_cv_create_t image_cv_create;
static image_cv_create2_t image_cv_create2;
static image_cv_create_from_data_t image_cv_create_from_data;
static image_cv_clone_t image_cv_clone;
static image_cv_width_t image_cv_width;
static image_cv_height_t image_cv_height;
static image_cv_depth_bytes_t image_cv_depth_bytes;
static image_cv_channels_t image_cv_channels;
static image_cv_format_t image_cv_format;
static image_cv_data_t image_cv_data;
static image_cv_data_size_t image_cv_data_size;
static image_cv_resize_t image_cv_resize;
static image_cv_add_alpha_channel_t image_cv_add_alpha_channel;
static image_cv_2d_rotate_t image_cv_2d_rotate;
static image_cv_3d_rotate_t image_cv_3d_rotate;
static image_cv_trim_t image_cv_trim;
static image_cv_set_alpha_t image_cv_set_alpha;
static image_cv_erase_t image_cv_erase;
static image_cv_show_t image_cv_show;
static image_cv_save_t image_cv_save;
static image_cv_free_t image_cv_free;

/* Image dynamic library handle. */
static DLIB *lib;

#define IMG_LOAD_CV_API(func) \
{ \
    func = dlib_symbol(lib, #func); \
    if(func == NULL) { \
        sg_log(LL_FATAL, #func" load failure."); \
        return -1; \
    } \
}


int image_init(void)
{
    char dlib_path[1024];

    module_dir(dlib_path, 1024);
    strcat(dlib_path, "image_cv.so");

    lib = dlib_open(dlib_path);
    if(lib == NULL) {
        sg_log(LL_FATAL, "OpenCV module load failure.");
        return -1;
    }

    IMG_LOAD_CV_API(image_cv_load);
    IMG_LOAD_CV_API(image_cv_create);
    IMG_LOAD_CV_API(image_cv_create2);
    IMG_LOAD_CV_API(image_cv_create_from_data);
    IMG_LOAD_CV_API(image_cv_clone);
    IMG_LOAD_CV_API(image_cv_width);
    IMG_LOAD_CV_API(image_cv_height);
    IMG_LOAD_CV_API(image_cv_depth_bytes);
    IMG_LOAD_CV_API(image_cv_channels);
    IMG_LOAD_CV_API(image_cv_format);
    IMG_LOAD_CV_API(image_cv_data);
    IMG_LOAD_CV_API(image_cv_data_size);
    IMG_LOAD_CV_API(image_cv_resize);
    IMG_LOAD_CV_API(image_cv_add_alpha_channel);
    IMG_LOAD_CV_API(image_cv_2d_rotate);
    IMG_LOAD_CV_API(image_cv_3d_rotate);
    IMG_LOAD_CV_API(image_cv_trim);
    IMG_LOAD_CV_API(image_cv_set_alpha);
    IMG_LOAD_CV_API(image_cv_erase);
    IMG_LOAD_CV_API(image_cv_show);
    IMG_LOAD_CV_API(image_cv_save);
    IMG_LOAD_CV_API(image_cv_free);

    if(image_sdl_init() != 0)
        return -1;

    sg_log_inf("Image library init succeed.");
    return 0;
}

struct image *image_load(const char *path)
{
    struct image *res;
    image_cv *im_cv;

    assert(path);
    assert(strlen(path) > 0);

    im_cv = image_cv_load(path);
    if(im_cv == NULL)
        return NULL;

    fprintf(stderr, "%d.\n", image_cv_width(im_cv));

    res = image_create_bind_cv_mat(im_cv);
    if(res == NULL) {
        image_cv_free(&im_cv);
        return NULL;
    }

    return res;
}

struct image *image_create(uint32_t w, uint32_t h, struct rgba8888 color)
{
    image_cv *im_cv;
    struct image *res;

    assert(w > 0);
    assert(h > 0);

    im_cv = image_cv_create(w, h, color);
    if(im_cv == NULL)
        return NULL;

    res = image_create_bind_cv_mat(im_cv);
    if(res == NULL) {
        image_cv_free(&im_cv);
        return NULL;
    }

    return res;
}

struct image *image_create_from_sdl_surface(image_sdl *src)
{
    image_cv *im_cv;
    struct image *res;

    assert(src);

    im_cv = sdl_surface_to_cv_mat((SDL_Surface *)src);
    if(im_cv == NULL)
        return NULL;

    res = image_create_bind_cv_mat(im_cv);
    if(res == NULL) {
        image_cv_free(&im_cv);
        return NULL;
    }

    return res;
}

struct image *image_create_from_cv_mat(image_cv *src)
{
    struct image *res;
    image_cv *img_cv_clone;

    assert(src);

    res = malloc(sizeof(struct image));
    if(res == NULL) {
        sg_log_err("struct image malloc failure.");
        return NULL;
    }
    memset(res, 0, sizeof(struct image));

    img_cv_clone = image_cv_clone(src);
    if(img_cv_clone == NULL) {
        sg_log_err("Image opencv create failure.");
        free(res);
        return NULL;
    }

    if(image_flush_instance(res, img_cv_clone) != 0) {
        sg_log_err("struct image bind image_cv failure.");
        image_cv_free(&img_cv_clone);
        free(res);
        res = NULL;
    }

    return res;
}

struct image *image_create_bind_cv_mat(image_cv *src)
{
    struct image *res;

    assert(src);

    res = malloc(sizeof(struct image));
    if(res == NULL) {
        sg_log_err("struct image malloc failure.");
        return NULL;
    }
    memset(res, 0, sizeof(struct image));

    if(image_flush_instance(res, src) != 0) {
        sg_log_err("struct image bind image_cv failure.");
        free(res);
        res = NULL;
    }

    return res;
}

struct image *image_create_from_text(const char *text, enum text_encoding enc,
                                     struct rgba8888 color,  const char *font_name,
                                     uint32_t font_size, int font_style)
{
    SDL_Surface *surf;

    assert(text);
    assert(strlen(text) > 0);
    assert(font_name);
    assert(strlen(font_name) > 0);
    assert(font_size > 0);

    surf = image_sdl_create_from_text(text, enc, color,
                                    font_name, font_size, font_style);

    if(surf == NULL)
        return NULL;

    return image_create_from_sdl_surface((image_sdl *)surf);
}

struct image *image_clone(struct image *src)
{
    assert(src);
    assert(src->ins);

    return image_create_from_cv_mat(src->ins);
}

int image_draw_text_in(const char *text, enum text_encoding enc,
                       struct rgba8888 color, int use_padding_bottom,
                       int32_t x, int32_t y, int32_t padding_bottom,
                       const char *font_name, uint32_t font_size,
                       int font_style, struct image *dst)
{
    int ret;
    image_cv *img_cv_drawn = NULL;
    SDL_Surface *dst_surf = NULL;

    assert(dst);
    assert(dst->ins);

    dst_surf = cv_mat_to_sdl_surface(dst->ins);
    if (dst_surf == NULL)
        return -1;

    if(use_padding_bottom)
        ret = image_sdl_draw_text_magnetic(text, enc, color, x, padding_bottom,
                                           font_name, font_size,
                                           font_style, dst_surf);
    else
        ret = image_sdl_draw_text(text, enc, color, x, y,
                                  font_name, font_size,
                                  font_style, dst_surf);
    if (ret != 0)
        goto end;

    img_cv_drawn = sdl_surface_to_cv_mat(dst_surf);
    if(img_cv_drawn == NULL) {
        ret = -1;
        goto end;
    }

    ret = image_flush_instance(dst, img_cv_drawn);
    if(ret != 0)
        image_cv_free(&img_cv_drawn);

    end:
    SAFE_SDL_FREESURFACE(dst_surf);
    return ret;
}

int image_draw_text(const char *text, enum text_encoding enc,
                    struct rgba8888 color,
                    int32_t x, int32_t y,
                    const char *font_name, uint32_t font_size,
                    int font_style, struct image *dst)
{
    return image_draw_text_in(text, enc, color, 0, x, y, 0,
                              font_name, font_size,
                              font_style, dst);
}

int image_draw_text_magnetic(const char *text, enum text_encoding enc,
                             struct rgba8888 color,
                             int32_t x, int32_t padding_bottom,
                             const char *font_name, uint32_t font_size,
                             int font_style, struct image *dst)
{
    return image_draw_text_in(text, enc, color, 1, x, 0, padding_bottom,
                              font_name, font_size,
                              font_style, dst);
}

int image_resize(struct image *src, uint32_t new_w, uint32_t new_h)
{
    int ret;
    image_cv *img_resized;

    assert(src);
    assert(src->ins);
    assert(new_w > 0);
    assert(new_h > 0);

    img_resized = image_cv_resize(src->ins, new_w, new_h);
    if(img_resized == NULL)
        return NULL;

    ret = image_flush_instance(src, img_resized);
    if(ret != 0)
        image_cv_free(&img_resized);

    return ret;
}

int image_2d_rotate(struct image *src, float angle)
{
    int ret;
    image_cv *rotated_img;

    assert(src);
    assert(src->ins);

    rotated_img = image_cv_2d_rotate(src->ins, angle);
    if(rotated_img == NULL)
        return NULL;

    ret = image_flush_instance(src, rotated_img);
    if(ret != 0)
        image_cv_free(&rotated_img);

    return ret;
}

int image_3d_rotate(struct image *src, float angle)
{
    int ret;
    image_cv *rotated_img;

    assert(src);
    assert(src->ins);
    assert(angle >= -90.00 && angle <= 90.00);

    rotated_img = image_cv_3d_rotate(src->ins, angle);
    if(rotated_img == NULL)
        return NULL;

    ret = image_flush_instance(src, rotated_img);
    if(ret != 0)
        image_cv_free(&rotated_img);

    return ret;
}

int image_mask(struct image *overlay, int32_t overlay_x, int32_t overlay_y,
               struct image *underlay, enum mask_style style)
{
    int ret = -1;
    SDL_Surface *overlay_surf = NULL;
    SDL_Surface *underlay_surf = NULL;
    image_cv *img_cv_mask = NULL;

    assert(overlay);
    assert(overlay->ins);
    assert(underlay);
    assert(underlay->ins);

    overlay_surf = cv_mat_to_sdl_surface(overlay->ins);
    if(overlay_surf == NULL)
        return -1;

    underlay_surf = cv_mat_to_sdl_surface(underlay->ins);
    if(underlay_surf == NULL) {
        ret = -1;
        goto end;
    }

    ret = image_sdl_mask(overlay_surf, overlay_x, overlay_y, underlay_surf, style);
    if(ret != 0)
        goto end;

    img_cv_mask = sdl_surface_to_cv_mat(underlay_surf);
    if(img_cv_mask == NULL) {
        ret = -1;
        goto end;
    }

    ret = image_flush_instance(underlay, img_cv_mask);
    if(ret != 0)
        image_cv_free(&img_cv_mask);

    end:
    SAFE_SDL_FREESURFACE(overlay_surf);
    SAFE_SDL_FREESURFACE(underlay_surf);

    return ret;
}

int image_set_alpha(struct image *src, uint8_t alpha)
{
    int ret;
    image_cv *img_cv_alpha;
    float alpha_percentage;

    assert(src);
    assert(src->ins);

    alpha_percentage = (float)alpha / (float)255;

    img_cv_alpha = image_cv_set_alpha(src->ins, alpha_percentage);
    if(img_cv_alpha == NULL)
        return -1;

    ret = image_flush_instance(src, img_cv_alpha);
    if(ret != 0)
        image_cv_free(&img_cv_alpha);

    return ret;
}

int image_add_alpha_channel(struct image *src)
{
    int ret;
    image_cv *img_cv_add_alpha;

    assert(src);
    assert(src->ins);

    if(image_cv_channels(src->ins) == 4)
        return 0;

    img_cv_add_alpha = image_cv_add_alpha_channel(src->ins);
    if(img_cv_add_alpha == NULL)
        return -1;

    ret = image_flush_instance(src, img_cv_add_alpha);
    if(ret != 0)
        image_cv_free(&img_cv_add_alpha);

    return ret;
}

int image_erase(struct image *src)
{
    assert(src);
    assert(src->ins);

    if(image_cv_erase(src->ins) != 0)
        return -1;

    src->data = image_cv_data(src->data);
    src->data_size = image_cv_data_size(src->ins);
    if(src->data == NULL || src->data_size == 0)
        return -1;

    return 0;
}

void image_show(const char *window_title, struct image *img)
{
    assert(window_title);
    assert(img);
    assert(img->ins);

    image_cv_show(window_title, img->ins);
}

int image_save(struct image *src, const char *path)
{
    assert(src);
    assert(src->ins);
    assert(path);
    assert(strlen(path) > 0);

    return image_cv_save(src->ins, path);
}

void image_free(struct image **img)
{
    assert(img);

    if(*img == NULL)
        return;

    if((*img)->ins)
        image_cv_free(&((*img)->ins));

    free(*img);
    *img = NULL;
}

SDL_Surface *cv_mat_to_sdl_surface(image_cv *src)
{
    int w, h, depth_bytes, ch;
    uint8_t *data;
    size_t size;
    enum pix_fmt pix_fmt;
    SDL_Surface *surf;

    assert(src);

    data = image_cv_data(src);
    w = image_cv_width(src);
    h = image_cv_height(src);
    depth_bytes = image_cv_depth_bytes(src);
    size = image_cv_data_size(src);
    ch = image_cv_channels(src);

    assert(data);
    assert(w > 0);
    assert(h > 0);
    assert(depth_bytes > 0);
    assert(size > 0);
    assert(ch == 3 || ch == 4);

    /* Pixel format of OpenCV mat must be BGR(A). */
    pix_fmt = image_cv_format(src);
    assert(pix_fmt == PIXFMT_BGR888 || pix_fmt == PIXFMT_BGRA8888);
    if(pix_fmt == PIXFMT_BGR888)
        ch = 3;
    else if(pix_fmt == PIXFMT_BGRA8888)
        ch = 4;
    else {
        sg_log_err("Unsupported format %d.", pix_fmt);
        return NULL;
    }

    surf = image_sdl_create_from_data(w, h, depth_bytes, ch,
                                    data, size, pix_fmt);

    return surf;
}

image_cv *sdl_surface_to_cv_mat(SDL_Surface *src)
{
    int ch;
    enum pix_fmt pix_fmt;
    image_cv *res;

    assert(src);

    ch = image_sdl_channel(src);
    /* You can only use surface created by image_sdl_create*** as param src,
       otherwise, surface loaded from disk file could be other pixel formats. */
    switch (src->format->format) {
        case SDL_PIXELFORMAT_ARGB8888:
            pix_fmt = PIXFMT_BGRA8888;
            break;
        case SDL_PIXELFORMAT_ABGR8888:
            pix_fmt = PIXFMT_RGBA8888;
            break;
        case SDL_PIXELFORMAT_RGB24:
        case SDL_PIXELFORMAT_RGB888:
            pix_fmt = PIXFMT_BGR888;
            break;
        case SDL_PIXELFORMAT_BGR24:
        case SDL_PIXELFORMAT_BGR888:
            pix_fmt = PIXFMT_RGB888;
            break;
        default:
            sg_log_err("Unsupported SDL format %s.", SDL_GetPixelFormatName(src->format->format));
            return NULL;
    }

    res = image_cv_create_from_data(src->w, src->h,
                                    src->format->BitsPerPixel / 8, ch,
                                    src->pixels,
                                    src->w * src->h * src->format->BitsPerPixel / 8,
                                    pix_fmt);

    return res;
}

void image_uninit(void)
{
    image_sdl_uninit();
    dlib_close(&lib);
}

int image_flush_instance(struct image *dst, image_cv *instance)
{
    uint32_t w, h;
    uint8_t  ch, depth_bits, *data;
    uint64_t data_size;
    image_cv *ins_orig;
    enum pix_fmt pix_fmt;

    assert(dst);
    assert(instance);
    if(image_cv_data(instance) == NULL)
        return -1;

    ins_orig = dst->ins;

    w = image_cv_width(instance);
    h = image_cv_height(instance);
    ch = image_cv_channels(instance);
    depth_bits = image_cv_depth_bytes(instance) * 8;
    data = image_cv_data(instance);
    data_size = image_cv_data_size(instance);
    pix_fmt = image_cv_format(instance);

    if(w == 0 || h == 0 || ch < 3 || ch > 4
       || depth_bits == 0
       || data == NULL || data_size == 0
       || pix_fmt < PIXFMT_MIN || pix_fmt > PIXFMT_MAX)
        return -1;

    dst->w = w;
    dst->h = h;
    dst->ch = ch;
    dst->depth_bits = depth_bits;
    dst->data = data;
    dst->data_size = data_size;
    dst->ins = instance;
    dst->pix_fmt = pix_fmt;

    if(ins_orig)
        image_cv_free(&ins_orig);

    return 0;
}