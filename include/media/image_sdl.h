/*
 * image_sdl.h
 * Author: wangwei.
 * Image process library based on SDL2 / SDL2_image / SDL2_ttf.
 */

#ifndef IMGUTIL_IMAGE_SDL_H
#define IMGUTIL_IMAGE_SDL_H

#include <stdbool.h>
#include "../../libavutil/frame.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"
#include "../util/macro.h"
#include "color.h"
#include "image.h"

#define Y_BOTTOM 0xFFFFFFFF

/* For ffmpeg. */
#define SAFE_AV_FREE(P)						if (NULL != (P)) { av_free(P); (P) = NULL; }
#define SAFE_AV_FRAME_FREE(P)				if (NULL != (P)) { av_frame_free(&P); (P) = NULL; }
#define SAFE_SWS_FREECONTEXT(P)				if (NULL != (P)) { sws_freeContext(P); (P) = NULL; }
#define SAFE_AVIO_CLOSE(P)					if (NULL != (P)) { avio_close(P); (P) = NULL;}
#define SAFE_AVFORMAT_FREE_CONTEXT(P)		if (NULL != (P)) { if (P->pb && !(P->flags & AVFMT_NOFILE)) { avio_close(P->pb); } avformat_free_context(P); (P) = NULL; }
#define SAFE_AVFORMAT_CLOSE_INPUT(P)		if (NULL != (P)) { avformat_close_input(P); }
#define SAFE_AVCODEC_CLOSE(P)				if (NULL != (P)) { avcodec_close(P); (P) = NULL; }
#define SAFE_AV_BITSTREAM_FILTER_CLOSE(P)	if (NULL != (P)) { av_bitstream_filter_close(P); (P) = NULL; }
#define SAFE_AV_DICT_FREE(P)				if (NULL != (P)) { av_dict_free(&P); (P) = NULL; }

/* For SDL2. */
#define SAFE_SDL_FREESURFACE(P)				if (NULL != (P)) { SDL_FreeSurface(P); (P) = NULL; }

/* Initialize image library. */
int image_sdl_init(void);

/* Create an RGBA image with given size and color, if you need totally
   transparent image, r/g/b/a all should be 0. */
SDL_Surface *image_sdl_create(uint32_t width, uint32_t height, struct rgba8888 color);

/* Create an RGB image with given size and color. */
SDL_Surface *image_sdl_create_3ch(uint32_t width, uint32_t height, struct rgb888 color);

/* Create an image with given rgb raw data buffer, the buffer will be quoted. */
SDL_Surface *image_sdl_create_from_rgb(void *rgb_buf, uint32_t width,
                                       uint32_t height, uint8_t depth);

/* Create an image with given rgba raw data buffer, the buffer will be quoted. */
SDL_Surface *image_sdl_create_from_rgba(void *rgba_buf, uint32_t width,
                                        uint32_t height, uint8_t depth);

/* Create an image with given raw data buffer, the buffer will be cloned. */
SDL_Surface *image_sdl_create_from_data(uint32_t w, uint32_t h,
                                        uint8_t depth_bytes, uint8_t ch,
                                        uint8_t *data, size_t size,
                                        enum pix_fmt pix_fmt);

/* Create an image with given text and style.
 * If specify_width is smaller than 1, means do not specify image width. */
SDL_Surface *image_sdl_create_from_text(const char *text, enum text_encoding enc,
                                        struct rgba8888 color, const char *font_name,
                                        uint32_t font_size, int font_style);

/* Clone new image from source image. */
SDL_Surface *image_sdl_clone(const SDL_Surface *src);

/* Malloc and clone raw rgba data of SDL_Surface. */
int image_sdl_copy_to_rgb(SDL_Surface *surf, char **out_buf,
                          size_t *out_size);

/* Load disk image file. */
SDL_Surface *image_sdl_load(const char *path);

/* Trim image with specified pixels. */
SDL_Surface *image_sdl_trim(SDL_Surface *src, uint32_t left,
                            uint32_t top, uint32_t right,
                            uint32_t bottom);

/* Mask an image to another. */
int image_sdl_mask(SDL_Surface *overlay, int32_t overlay_x,
                   int32_t overlay_y, SDL_Surface *dst_image,
                   enum mask_style style);

/* Detect PNG format.
 * Return 1: is PNG file.
 *        0: other format.
 *       -1: error. */
int image_sdl_check_png(const char *png_path);

int image_sdl_channel(SDL_Surface *src);

/* Output image information. */
void image_sdl_printf_info(const SDL_Surface *src);

/* Write on the image. */
int image_sdl_draw_text(const char *text, enum text_encoding enc,
                        struct rgba8888 color,
                        int32_t x, int32_t y,
                        const char *font_name, uint32_t font_size,
                        int font_style, SDL_Surface *draw_dst);

/* Write on the image with distance to bottom. */
int image_sdl_draw_text_magnetic(const char *text, enum text_encoding enc,
                                 struct rgba8888 color,
                                 int32_t x, int32_t padding_bottom,
                                 const char *font_name, uint32_t font_size,
                                 int font_style, SDL_Surface *draw_dst);

/* Set image alpha degree, and return new image with new alpha degree.
   It will not change alpha value of 'src' image. */
SDL_Surface *image_sdl_set_alpha(SDL_Surface *src, uint8_t alpha);

/* Set image data to transparent. */
int image_sdl_erase(SDL_Surface *src);

/* Zoom an image. */
SDL_Surface *image_sdl_zoom(SDL_Surface *src, double dst_scale_x,
                            double dst_scale_y);

/* Resize an image. */
SDL_Surface *image_sdl_resize(SDL_Surface *src, uint32_t new_width,
                              uint32_t new_height);

/* Save image to disk. */
int image_sdl_save(SDL_Surface *src, const char *path,
                   enum img_fmt fmt);

/* Get image pixel data size, returns -1 if error. */
int image_sdl_pixel_size(const SDL_Surface *src);

/* Convert SDL image to ffmpeg frame. */
AVFrame *image_sdl_surface_to_frame(const SDL_Surface *src);

/* Create new SDL image from ffmpeg frame. */
SDL_Surface *image_sdl_surface_from_frame(const AVFrame *src);

/* Free image library. */
void image_sdl_uninit(void);

#endif //IMGUTIL_IMAGE_SDL_H