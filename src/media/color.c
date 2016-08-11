/*
 * color.c
 * Author: wangwei.
 * Color process library.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#if defined(__linux__)
# include <stdint-gcc.h>
#endif
#include "../util/assert.h"
#include "../util/string.h"
#include "color.h"


int hex_str_to_rgb888(const char *str, struct rgb888 *rgb)
{
    unsigned long int num;
    unsigned long start_pos = 0;
    char *start_str;
    char *endptr = NULL;

    assert(str);
    assert(strlen(str) > 0);
    assert(rgb);

    /* Ingore '#'. */
    start_str = strstr(str, "#");
    if(start_str)
        start_pos = start_str - str + 1;
    else
        start_pos = 0;
    if(start_pos > 1)
        return -1;
    if(strlen(&str[start_pos]) == 0)
        return -1;

    /* Check hex string. */
    if(!is_hex_str(&str[start_pos]))
        return -1;

    /* Convert. */
    num = strtoul(&str[start_pos], &endptr, 16);
    if(endptr == &str[strlen(str)]){
        rgb->r = num >> 16;
        rgb->g = num >> 8;
        rgb->b = num >> 0;
        return 0;
    }

    return -1;
}

int rgb888_to_hex_str(struct rgb888 *rgb, char *buf,
                      size_t buf_len, int pound_first,
                      int uppercase)
{
    uint32_t color_num = 0;

    assert(rgb);
    assert(buf);
    assert(buf_len >= ((pound_first == 1) ? 8 : 7));

    color_num += rgb->r << 16;
    color_num += rgb->g << 8;
    color_num += rgb->b << 0;

    snprintf(buf, buf_len, "%s%06x", (pound_first == 1) ? "#" : "", color_num);
    if(uppercase == 1)
        str_to_upper(buf);
    return 0;
}