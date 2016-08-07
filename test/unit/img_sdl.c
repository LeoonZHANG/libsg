/*
 * img_sdl.c
 * Author: wangwei.
 * Test for image util module img_sdl.
 */

#include "../imgutil/img_sdl.h"

/* Print image format info. */
void sdl_print_fmt(const char *path)
{
    SDL_Surface *s = img_sdl_load(path);
    if(s) {
        printf("%s format:%s, ch:%d.\n", path, SDL_GetPixelFormatName(s->format->format), img_sdl_channel(s));
        SDL_FreeSurface(s);
    }
}

/* Test for different PNG formats. */
void test_print_fmt(void)
{
    sdl_print_fmt("test png/24bit.png");
    sdl_print_fmt("test png/32bit.png");
    sdl_print_fmt("test png/4x100.fw.png");
    sdl_print_fmt("test png/5x100.fw.png");
    sdl_print_fmt("test png/100x4.fw.png");
    sdl_print_fmt("test png/100x5.fw.png");
    sdl_print_fmt("test png/original.fw.png");
    sdl_print_fmt("test png/8bit-2clr-alptrans.png");
    sdl_print_fmt("test png/8bit-2clr-idxtrans.png");
    sdl_print_fmt("test png/8bit-2clr-untrans.png");
    sdl_print_fmt("test png/8bit-32clr-alptrans.png");
    sdl_print_fmt("test png/8bit-32clr-idxtrans.png");
    sdl_print_fmt("test png/8bit-32clr-untrans.png");
    sdl_print_fmt("test png/8bit-256clr-aphtrans.png");
    sdl_print_fmt("test png/8bit-256clr-idxtrans.png");
    sdl_print_fmt("test png/8bit-256clr-untrans.png");
}

int main(int argc, char **argv)
{
    test_print_fmt();
    return 0;
}