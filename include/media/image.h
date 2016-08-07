/*
 * image.h
 * Author: wangwei.
 * Image process library.
 */

#ifndef IMGUTIL_IMAGE_H
#define IMGUTIL_IMAGE_H

#include "color.h"

typedef void image_cv;
typedef void image_sdl;

/* Text character encoding. */
enum text_encoding {
    TEXTENCODING_UNKNOWN = 0,
    TEXTENCODING_ANSI    = 1,
    TEXTENCODING_UTF8    = 2,
    TEXTENCODING_UNICODE = 3
};

/* Image encoding formats. */
enum img_fmt {
    IMGFMT_NONE = -1,
    IMGFMT_RAW = 0,
    IMGFMT_BMP = 1,
    IMGFMT_JPG = 2,
    IMGFMT_GIF = 3,
    IMGFMT_PNG = 4,
    IMGFMT_TIF = 5
};

/* Pixel formats supported for now in this library. */
enum pix_fmt {
    PIXFMT_MIN      = 0,
    PIXFMT_RGB888   = 0,
    PIXFMT_BGR888   = 1,
    PIXFMT_RGBA8888 = 2,
    PIXFMT_BGRA8888 = 3,
    PIXFMT_MAX      = 3
};

/* How to mask an image to another. */
enum mask_style {
    /* Blend style mask, if overlay image has transparent background,
     * destination image will not be replaced by transparent pixels
     * from overlay image. */
    MASKSTYLE_BLEND = 1,
    /* Replace style mask, destination image will be replaced by
     * overlay image, even if overlay image has transparent pixels. */
    MASKSTYLE_REPLACE = 2
};

/* Image type structure. */
struct image {
    uint32_t w;             /* Width. */
    uint32_t h;             /* Height. */
    uint8_t  ch;            /* Channel count. */
    uint8_t  depth_bits;    /* Bits per pixel. */
    uint8_t  *data;         /* Data pointer. */
    uint64_t data_size;     /* Data size. */
    image_cv   *ins;        /* OpenCV cv::Mat instance pointer. */
    enum pix_fmt pix_fmt;   /* Pixel format. */
};

/* Initialize image library. */
int image_init(void);

/* Load file from disk. */
struct image *image_load(const char *path);

/* Create an image with given size and color, if you need totally
   transparent image, r/g/b/a all should be 0. */
struct image *image_create(uint32_t w, uint32_t h, struct rgba8888 color);

/* Create blank image from raw image data. */
struct image *image_create_from_data(uint32_t w, uint32_t h,
                                     uint8_t depth_bytes, uint8_t ch,
                                     uint8_t *data, enum pix_fmt pix_fmt);

/* Create an 4 channels image with given text and style.
   If specify_width is smaller than 1, means do not specify image width. */
struct image *image_create_from_text(const char *text, enum text_encoding enc,
                                     struct rgba8888,  const char *font_name,
                                     uint32_t font_size, int font_style);

/* Create an struct image from SDL surface image,
   the image raw data will be cloned. */
struct image *image_create_from_sdl_surface(image_sdl *surface);

/* Create an struct image from OpenCV cv::Mat image,
   the cv::Mat will be cloned. */
struct image *image_create_from_cv_mat(image_cv *src);

/* Create an struct image from OpenCV cv::Mat image,
   the cv::Mat will not be quoted. */
struct image *image_create_bind_cv_mat(image_cv *mat);

/* Clone an image. */
struct image *image_clone(struct image *src);

/* Write on the image. */
int image_draw_text(const char *text, enum text_encoding enc,
                    struct rgba8888 color,
                    int32_t x, int32_t y,
                    const char *font_name, uint32_t font_size,
                    int font_style, struct image *dst);

/* Write on the image with distance to bottom. */
int image_draw_text_magnetic(const char *text, enum text_encoding enc,
                             struct rgba8888 color,
                             int32_t x, int32_t padding_bottom,
                             const char *font_name, uint32_t font_size,
                             int font_style, struct image *dst);

/* Resize an image. */
int image_resize(struct image *src, uint32_t new_w, uint32_t new_h);

/* Rotate an image in 2D space with clockwise degree, which starts with 0. */
int image_2d_rotate(struct image *src, float angle);

/*
 Function: rotate an image in 3D space with given angle.
 Param: angle: angle with 3D overlooking perspective between -90 and 90 degree.
    -90 degree:           0 degree:            90 degree:
              |                Back                     |
              |                ----------               |
        Front | Back           Front               Back | Front

 Return 0: rotate succeed, otherwise failed.
*/
int image_3d_rotate(struct image *src, float angle);

/* Mask an image to another, the underlay is the output result image. */
int image_mask(struct image *overlay, int32_t overlay_x, int32_t overlay_y,
               struct image *underlay, enum mask_style style);

/* Set image alpha degree, and return new image with new alpha degree.
   It will not change alpha value of 'src' image. */
int image_set_alpha(struct image *src, uint8_t alpha);

/* Insert alpha channel for an image. */
int image_add_alpha_channel(struct image *src);

/* Clean an image as blank transparent image. */
int image_erase(struct image *src);

/* Display image in new window. */
void image_show(const char *window_title, struct image *img);

/* Save image to disk.
   Output image format depends on the extension of path string. */
int image_save(struct image *src, const char *path);

/* Free image. */
void image_free(struct image **img);

/* Free image library. */
void image_uninit(void);

#endif //IMGUTIL_IMAGE_H