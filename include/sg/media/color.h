/*
 * color.h
 * Author: wangwei.
 * Color process library.
 */

#ifndef IMGUTIL_COLOR_H
#define IMGUTIL_COLOR_H

#include <sg/sg.h>

enum color_space {
    COLORSPACE_RGB888   = 0,
    COLORSPACE_BGR888   = 1,
    COLORSPACE_RGBA8888 = 2,
    COLORSPACE_BGRA8888 = 3
};

/* 24-bit depth RGB color. */
struct rgb888{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

/* 32-bit depth RGBA color. */
struct rgba8888{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

/* Black. */
static const struct rgb888 rgb_black = {0x00, 0x00, 0x00};

/* White. */
static const struct rgb888 rgb_white = {0xFF, 0xFF, 0xFF};

/* Convert html like color string (such as #1A2B3C) to RGB value. */
int hex_str_to_rgb888(const char *str, struct rgb888 *rgb);

/* Convert RGB value to html like color string (such as #1A2B3C). */
int rgb888_to_hex_str(struct rgb888 *rgb, char *buf,
                      size_t buf_len, int pound_first,
                      int uppercase);

#endif //IMGUTIL_COLOR_H