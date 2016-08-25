/*
 * image_cv.cpp
 * Author: wangwei.
 * C style image process APIs based on OpenCV 2.4 (C++ APIs).
 */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stdint.h>
#if defined(__linux__)
# include <stdint-gcc.h>
#endif
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "../util/number.h"
#include "../util/assert.h"
#include "image_cv.h"

/* using namespace cv; */

image_cv *image_cv_load(const char *path)
{
    cv::Mat *res, swap;

    assert(path);
    assert(strlen(path) > 0);

    swap = cv::imread(path, CV_LOAD_IMAGE_UNCHANGED);
    if(NULL == swap.data) {
        fprintf(stderr, "imread error.");
        return NULL;
    }

    res = new cv::Mat(swap);

    return (image_cv *)res;
}

image_cv *image_cv_create(uint32_t w, uint32_t h, struct rgba8888 color)
{
    cv::Mat *mat;
    int depth_bits = 4 * 8;
    int ch = 4;

    assert(w > 0);
    assert(h > 0);

    mat = new cv::Mat(cv::Size(w, h), CV_MAKETYPE(depth_bits, ch),
                      cv::Scalar(color.b, color.g, color.r, color.a));

    return (image_cv *)mat;
}

image_cv *image_cv_create2(uint32_t w, uint32_t h, int depth_bytes, int ch)
{
    cv::Mat *mat;
    int depth_bits;

    assert(w > 0);
    assert(h > 0);
    assert(depth_bytes > 0);
    assert(ch == 3 || ch == 4);

    depth_bits = depth_bytes * 8;
    mat = new cv::Mat(cv::Size(w, h), CV_MAKETYPE(depth_bits, ch));

    return (image_cv *)mat;
}

image_cv *image_cv_create_from_data(uint32_t w, uint32_t h,
                                    uint8_t depth_bytes, uint8_t ch,
                                    uint8_t *data, uint64_t size,
                                    enum pix_fmt pix_fmt)
{
    cv::Mat *res;
    uint64_t size_in;
    unsigned char swap;
    size_t i;

    assert(w > 0);
    assert(h > 0);
    assert(depth_bytes > 0);
    assert(ch == 3 || ch == 4);
    assert(data);
    assert(pix_fmt >= PIXFMT_MIN || pix_fmt <= PIXFMT_MAX);

    size_in = w * h * depth_bytes;
    assert(size_in == size);

    res = (cv::Mat *)image_cv_create2(w, h, depth_bytes, ch);
    if(res)
        memcpy(res->data, data, size);

    /* OpenCV use BGR(A) storage format, convert from RGB(A). */
    if(pix_fmt == PIXFMT_RGB888 || pix_fmt == PIXFMT_RGBA8888)
        for(i = 0; i < size; i += ch) {
            swap = res->data[i];
            res->data[i] = res->data[i + 2];
            res->data[i + 2] = swap;
        }

    return res;
}

image_cv *image_cv_clone(image_cv *src)
{
    cv::Mat *res;

    res = new cv::Mat();
    *res = ((cv::Mat *)src)->clone();

    return res;
}

size_t image_cv_width(image_cv *img)
{
    cv::Mat *mat = (cv::Mat *)img;

    assert(mat);

    return mat->cols;
}

size_t image_cv_height(image_cv *img)
{
    cv::Mat *mat = (cv::Mat *)img;

    assert(mat);

    return mat->rows;
}

int image_cv_depth_bytes(image_cv *img)
{
    assert(img);

    /* OpenCV use BGR(A)888(8) pixel format by default. */
    return ((cv::Mat *)img)->channels();
}

int image_cv_channels(image_cv *img)
{
    assert(img);

    return ((cv::Mat *)img)->channels();
}

enum pix_fmt image_cv_format(image_cv *img)
{
    cv::Mat *mat = (cv::Mat *)img;

    assert(mat);

    if(mat->channels() == 3)
        return PIXFMT_BGR888;
    else
        return PIXFMT_BGRA8888;
}

uint8_t *image_cv_data(image_cv *img)
{
    cv::Mat *mat = (cv::Mat *)img;

    assert(mat);

    return mat->data;
}

size_t image_cv_data_size(image_cv *img)
{
    cv::Mat *mat = (cv::Mat *)img;

    assert(mat);

    return (size_t)(mat->cols * mat->rows * mat->elemSize());
}

image_cv *image_cv_resize(image_cv *src, uint32_t new_w, uint32_t new_h)
{
    cv::Mat *dst;

    assert(src);
    assert(new_w > 0);
    assert(new_h > 0);

    dst = new cv::Mat();
    cv::resize(*((cv::Mat *)src), *dst, cv::Size(new_w, new_h),
               0, 0, CV_INTER_AREA);

    return dst;
}

image_cv *image_cv_add_alpha_channel(image_cv *src)
{
    cv::Mat *dst;
    cv::Mat channels[4]; /* B->G->R->A channels. */

    if(image_cv_channels(src) == 4)
        return image_cv_clone(src);

    dst = new cv::Mat();
    if(dst == NULL)
        return NULL;

    cv::split(*((cv::Mat *)src), channels);
    channels[3] = channels[0] + 255;
    cv::merge(channels, 4, *dst);

    return dst;
}

image_cv *image_cv_2d_rotate(image_cv *src, float angle)
{
    cv::Mat *src_mat = (cv::Mat *)src;
    cv::Mat rotate_matrix;
    cv::Mat *rotate_img;
    cv::Point2f rotate_center; /* Rotate center point. */
    double scale = 1; /* Do not resize. */

    assert(src_mat);

    rotate_img = new cv::Mat();
    if(rotate_img == NULL)
        return NULL;

    rotate_center = cv::Point2f(src_mat->cols / 2, src_mat->rows / 2);
    rotate_matrix = cv::getRotationMatrix2D(rotate_center, angle, scale);
    cv::warpAffine(*src_mat, *rotate_img, rotate_matrix, src_mat->size());

    return rotate_img;
}

image_cv *image_cv_3d_rotate(image_cv *src, float angle)
{
    int i;
    cv::Mat *src_img = (cv::Mat *)src, *dst;
    cv::Mat warp_matrix;
    /* Transform coordinate. */
    cv::Point2f src_quad[4], dst_quad[4];
    /* Destination image rows. */
    uint32_t dst_height;
    /* Vertical perspective height difference against source image. */
    uint32_t dst_height_diff;
    /* Vertical perspective height difference ratio against source image. */
    float dst_height_diff_ratio;

    assert(src_img);
    assert(angle >= -90.00); /* Min angle. */
    assert(angle <= 90.00); /* Max angle. */

    /* Tentative schedule, next step, should returns blank image if angle is -90.00 or 90.00. */
    if(float_cmp(angle, -90.00) == NUMCMP_EQUALS)
        angle = -89.99;
    if(float_cmp(angle, 90.00) == NUMCMP_EQUALS)
        angle = 89.99;

    dst_height_diff_ratio = 10.00 / 100.00;
    dst_height_diff = (src_img->rows + src_img->cols) * dst_height_diff_ratio;
    dst_height = dst_height_diff + src_img->rows;

    /* Source image perspective coordinate. */
    src_quad[0].x = 0; /* Top left. */
    src_quad[0].y = 0;
    src_quad[1].x = src_img->cols - 1; /* Top right. */
    src_quad[1].y = 0;
    src_quad[2].x = 0; /* Bottom left. */
    src_quad[2].y = src_img->rows - 1;
    src_quad[3].x = src_img->cols - 1; /* Bottom right. */
    src_quad[3].y = src_img->rows - 1;

    /* Destination image perspective coordinate. */
    dst_quad[0].x = src_img->cols * fabs(angle) / 180.00; /* Top left. */
    dst_quad[0].y = 0 - ((dst_height_diff / 2) * angle / 90.00);
    dst_quad[1].x = src_img->cols - dst_quad[0].x; /* Top right. */
    dst_quad[1].y = - dst_quad[0].y;
    dst_quad[2].x = dst_quad[0].x; /* Bottom left. */
    dst_quad[2].y = src_img->rows - dst_quad[0].y;
    dst_quad[3].x = dst_quad[1].x; /* Bottom right. */
    dst_quad[3].y = src_img->rows - dst_quad[1].y;

    /* Centre destination image. */
    for(i = 0; i < 4; i++)
        dst_quad[i].y += dst_height_diff / 2;

    /* Special angle coordinate, for best image quality. */
    /* if(float_is_zero(angle) == NUMCMP_EQUALS)
        memcpy(dst_quad, src_quad, sizeof(src_quad)); */

    dst = new cv::Mat(dst_height, src_img->cols,
                      CV_MAKETYPE(src_img->depth(), src_img->channels()));

    /* Transform. */
    warp_matrix = cv::getPerspectiveTransform(src_quad, dst_quad);
    cv::warpPerspective(*src_img, *dst, warp_matrix,
                        cv::Size(src_img->cols, dst_height));

    return (image_cv *)dst;
}

image_cv *image_cv_trim(image_cv *src, uint32_t left,
                        uint32_t top, uint32_t right,
                        uint32_t bottom)
{
    cv::Mat *res;
    cv::Mat *src_mat = (cv::Mat *)src;
    cv::Range rg_cols, rg_rows;

    assert((left + right) < (uint32_t)src_mat->cols);
    assert((top + bottom) < (uint32_t)src_mat->rows);

    rg_cols.start = left;
    rg_cols.end = src_mat->cols - right;
    rg_rows.start = top;
    rg_rows.end = src_mat->rows - bottom;

    res = new cv::Mat(*src_mat, rg_rows, rg_cols);

    return res;
}

image_cv *image_cv_set_alpha(image_cv *src, float alpha)
{
    cv::Mat *src_ptr;
    cv::Mat *src_with_alpha = NULL;
    cv::Mat *res;
    cv::Mat channels[4]; /* B->G->R->A channels. */

    assert(src);
    assert(alpha >= 0);

    if(image_cv_channels(src) == 3) {
        src_with_alpha = (cv::Mat *)image_cv_add_alpha_channel(src);
        src_ptr = src_with_alpha;
    }else
        src_ptr = (cv::Mat *)src;

    if(src_ptr == NULL)
        return NULL;

    res = new cv::Mat();

    /* Split channels to 4 Mats. */
    cv::split(*((cv::Mat *)src_ptr), channels);

    channels[3] = channels[3] * alpha; /* Change transparency percentage. */
    cv::merge(channels, 4, *res);

    if(src_with_alpha)
        image_cv_free((image_cv **)&src_with_alpha);

    return res;
}

int image_cv_erase(image_cv *src)
{
    cv::Mat *src_mat = (cv::Mat *)src;
    int data_size;

    assert(src);

    data_size = image_cv_data_size(src);
    if(data_size <= 0)
        return -1;

    memset(src_mat->data, 0, data_size);

    return 0;
}

void image_cv_show(const char *window_title, image_cv *img)
{
    cv::Mat *mat = (cv::Mat *)img;

    assert(mat);

    cv::namedWindow(window_title, CV_WINDOW_AUTOSIZE);
    cv::imshow(window_title, *mat);
    cv::waitKey(0);
}

int image_cv_save(image_cv *img, const char *path)
{
    cv::Mat *mat = (cv::Mat *)img;

    assert(mat);
    assert(path);
    assert(strlen(path) > 0);

    return cv::imwrite(path, *mat);
}

void image_cv_free(image_cv **img)
{
    cv::Mat **mat = (cv::Mat **)img;

    assert(mat);
    if(!*mat)
        return;

    delete *mat;
    *mat = NULL;
}