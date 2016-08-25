/*
 * image_cv.h
 * Author: wangwei.
 * C style image process APIs based on OpenCV 2.4 (C++ APIs).
 */

#ifndef IMGUTIL_IMAGE_CV_H
#define IMGUTIL_IMAGE_CV_H

#include "../util/macro.h"
#include "image.h"

#define IMAGECV_API extern "C"

/* Load image from disk. */
IMAGECV_API image_cv *image_cv_load(const char *path);

/* Create blank 4 channels image. */
IMAGECV_API image_cv *image_cv_create(uint32_t w, uint32_t h,
                                      struct rgba8888 color);

/* Create blank image by size, pixel depth and channels,
   if channel count is 4, result image is transparent. */
IMAGECV_API image_cv *image_cv_create2(uint32_t w, uint32_t h,
                                       int depth_bytes, int ch);

/* Create image from raw image data. */
IMAGECV_API image_cv *image_cv_create_from_data(uint32_t w, uint32_t h,
                        uint8_t depth_bytes, uint8_t ch,
                        uint8_t *data, uint64_t size,
                        enum pix_fmt pix_fmt);

/* Clone an image. */
IMAGECV_API image_cv *image_cv_clone(image_cv *src);

/* Get image width. */
IMAGECV_API size_t image_cv_width(image_cv *img);

/* Get image height. */
IMAGECV_API size_t image_cv_height(image_cv *img);

/* Get image pixel depth bytes. */
IMAGECV_API int image_cv_depth_bytes(image_cv *img);

/* Get image channel count. */
IMAGECV_API int image_cv_channels(image_cv *img);

/* Get image pixel format. */
IMAGECV_API enum pix_fmt image_cv_format(image_cv *img);

/* Get image raw data pointer. */
IMAGECV_API uint8_t *image_cv_data(image_cv *img);

/* Get image raw data size. */
IMAGECV_API size_t image_cv_data_size(image_cv *img);

/* Resize an image. */
IMAGECV_API image_cv *image_cv_resize(image_cv *src, uint32_t new_w, uint32_t new_h);

/* Insert alpha channel for BGR image. */
IMAGECV_API image_cv *image_cv_add_alpha_channel(image_cv *src);

/* Rotate an image in 2D space with clockwise degree. */
IMAGECV_API image_cv *image_cv_2d_rotate(image_cv *src, float angle);

/*
 Function: rotate an image in 3D space with given angle.
 Param angle: angle with 3D overlooking perspective between -90 and 90 degree.
    -90 degree:           0 degree:            90 degree:
              |                Back                     |
              |                ----------               |
        Front | Back           Front               Back | Front

 Return: rotated image, its height is bigger than source image.
*/
IMAGECV_API image_cv *image_cv_3d_rotate(image_cv *src, float angle);

/* Trim image with specified pixels. */
IMAGECV_API image_cv *image_cv_trim(image_cv *src, uint32_t left,
                                    uint32_t top, uint32_t right,
                                    uint32_t bottom);

/* Function: change transparency degree of an image.
   Param alpha: transparency percentage between 0.0 and 1.0,
   if alpha is bigger than 1.0, this interface works too,
   this is the way to increase opacity. */
IMAGECV_API image_cv *image_cv_set_alpha(image_cv *src, float alpha);

/* Set image to blank transparent image. */
IMAGECV_API int image_cv_erase(image_cv *src);

/* Display image in new window. */
IMAGECV_API void image_cv_show(const char *window_title, image_cv *img);

/* Save image to disk.
   Output image format depends on the extension of path string. */
IMAGECV_API int image_cv_save(image_cv *src, const char *path);

/* Free image. */
IMAGECV_API void image_cv_free(image_cv **img);

#endif //IMGUTIL_IMAGE_CV_H