/*
 * image_sdl.c
 * Author: wangwei.
 * Image process library based on SDL2 / SDL2_image / SDL2_ttf.
 */

#include <sys/stat.h>
#include "SDL2/SDL_surface.h"
#include "SDL2/SDL_endian.h"
#include "../../libavutil/avutil.h"
#include "../../libavcodec/avcodec.h"
#include "../util/log.h"
#include "../util/file.h"
#include "../util/assert.h"
#include "image.h"
#include "image_sdl.h"
#include "color.h"
#include "pixel.h"
#include "font.h"

/* SDL interprets each pixel as a 32-bit number, so our masks must depend
   on the endianness (byte order) of the machine. */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    #define rmask 0xff000000
    #define gmask 0x00ff0000
    #define bmask 0x0000ff00
    #define amask 0x000000ff
#else
	#define rmask 0x000000ff
	#define gmask 0x0000ff00
	#define bmask 0x00ff0000
	#define amask 0xff000000
#endif

struct rgba8888 color_transparent = {0, 0, 0, 0};
struct rgb888 color_black = {0, 0, 0};


int image_sdl_init(void)
{
	int flag;
	char str[128] = {0};

	/* Ensure use --enable-png when you install SDL2_image with source code. */
	flag = IMG_Init(IMG_INIT_PNG);
	if(flag == 0) {
		sg_log_err("IMG_Init failure, %s.", IMG_GetError());
		return -1;
	}
	if(flag & IMG_INIT_PNG)
		strcat(str, "PNG ");
	if(flag & IMG_INIT_JPG)
		strcat(str, "JPG ");
	if(flag & IMG_INIT_TIF)
		strcat(str, "TIF ");
	if(flag & IMG_INIT_WEBP)
		strcat(str, "WEBP ");

	flag = SDL_Init(0); /* Do not use SDL_INIT_VIDEO under text interface. */
	if(flag < 0) {
		sg_log_err("SDL_Init failure : %s.", SDL_GetError());
		return -1;
	}

	flag = font_lib_init();
	if(flag < 0) {
		sg_log_err("font_lib_init failure : %s.", TTF_GetError());
		return -1;
	}

	sg_log_inf("Image SDL init succeed, support format(s): %s.", str);
	return 0;
}

/* Create a 32-bit surface image, if you need totally
 * transparent image, r/g/b/a all should be 0. */
SDL_Surface *image_sdl_create(uint32_t width, uint32_t height,
							  struct rgba8888 color)
{
	/* Create a 32-bit surface with the bytes of
	 * each pixel in R,G,B,A order, as expected
	 * by OpenGL for textures. */
	SDL_Surface *surf;
	int rc;

	assert(width > 0);
	assert(height > 0);

    surf = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32,
								rmask, gmask, bmask, amask);
	if(surf == NULL) {
		sg_log_err("CreateRGBSurface %d*%d failure, %s.",
				   width, height, SDL_GetError());
		return NULL;
	}

	/* Transparent. */
	if(color.r == 0 && color.g == 0 && color.b == 0 && color.a == 0)
		return surf;

	rc = SDL_FillRect(surf, NULL,
					  SDL_MapRGBA(surf->format,
								  color.r, color.g, color.b, color.a));
	if(rc != 0) {
		sg_log_err("SDL_FillRect failure: %s.", SDL_GetError());
		SDL_FreeSurface(surf);
		surf = NULL;
	}

	return surf;
}

SDL_Surface *image_sdl_create_3ch(uint32_t width, uint32_t height,
								  struct rgb888 color)
{
	SDL_Surface *surf;
	int rc;

	assert(width > 0);
	assert(height > 0);

	surf = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32,
								rmask, gmask, bmask, 0);
	if(surf == NULL) {
		sg_log_err("CreateRGBSurface %d*%d failure, %s.",
				   width, height, SDL_GetError());
		return NULL;
	}

	/* Blank. */
	if(color.r == 0 && color.g == 0 && color.b == 0)
		return surf;

	rc = SDL_FillRect(surf, NULL,
					  SDL_MapRGB(surf->format,
								  color.r, color.g, color.b));
	if(rc != 0) {
		sg_log_err("SDL_FillRect failure: %s.", SDL_GetError());
		SDL_FreeSurface(surf);
		surf = NULL;
	}

	return surf;
}

SDL_Surface *image_sdl_create_from_rgb(void *rgb_buffer, uint32_t width,
									   uint32_t height, uint8_t depth)
{
	SDL_Surface *surf;

	assert(rgb_buffer);
	assert(width > 0);
	assert(height > 0);
	assert(depth > 0);

	/* It's much faster than SDL_CreateRGBSurface and SDL_ConvertSurface. */
	surf = SDL_CreateRGBSurfaceFrom(rgb_buffer, width, height, depth,
									width * ( depth / 8 /* Bits. */ ),
									rmask, gmask, bmask, 0);
	if(surf == NULL)
		sg_log_err("SDL_CreateRGBSurfaceFrom error, %s.", SDL_GetError());

	return surf;
}

SDL_Surface *image_sdl_create_from_rgba(void *rgba_buffer, uint32_t width,
										uint32_t height, uint8_t depth)
{
	SDL_Surface *surf;

	assert(rgba_buffer);
	assert(width > 0);
	assert(height > 0);
	assert(depth > 0);

	/* It's much faster than SDL_CreateRGBSurface and SDL_ConvertSurface. */
	surf = SDL_CreateRGBSurfaceFrom(rgba_buffer, width, height, depth,
									width * ( depth / 8 /* Bits. */ ),
									rmask, gmask, bmask, amask);
	if(surf == NULL)
		sg_log_err("SDL_CreateRGBSurfaceFrom error, %s.", SDL_GetError());

	return surf;
}

SDL_Surface *image_sdl_create_from_data(uint32_t w, uint32_t h,
										uint8_t depth_bytes, uint8_t ch,
										uint8_t *data, size_t size,
										enum pix_fmt pix_fmt)
{
	size_t i;
	SDL_Surface *surf;
	unsigned char swap;

	assert(data);
	assert(ch == 3 || ch == 4);
	assert(pix_fmt >= PIXFMT_MIN || pix_fmt <= PIXFMT_MAX);

	if(ch == 3)
		surf = image_sdl_create_3ch(w, h, color_black);
	else
		surf = image_sdl_create(w, h, color_transparent);
	if(surf == NULL)
		return NULL;
	memcpy(surf->pixels, data, size);

	/* Convert BGR(A) to RGB(A). */
	if(pix_fmt == PIXFMT_BGR888 || pix_fmt == PIXFMT_BGRA8888)
		for(i = 0; i < size; i += ch) {
			swap = ((uint8_t *)surf->pixels)[i];
			((uint8_t *)surf->pixels)[i] = ((uint8_t *)surf->pixels)[i + 2];
			((uint8_t *)surf->pixels)[i + 2] = swap;
		}

	return surf;
}

SDL_Surface *image_sdl_create_from_text(const char *text, enum text_encoding enc,
										struct rgba8888 color, const char *font_name,
										uint32_t font_size, int font_style)
{
	const SDL_Color RGB_Noname = {color.r, color.g, color.b, color.a};
	SDL_Surface 	*surf = NULL;
	TTF_Font		*font;

	assert(text);
	assert(enc != TEXTENCODING_UNKNOWN);
	assert(font_name);
	assert(strlen(font_name) > 0);
	assert(font_size < 1000);
	if(strlen(text) == 0)
		return NULL;

	/* Load font and draw text. */
	font = font_lib_seek(font_name, font_size);
	if(font == NULL) {
		sg_log_err("font_lib_get_font(%s, %d) returns null.",
				   font_name, font_size);
		return NULL;
	}
	font_set_style(font, font_style);
	if(enc == TEXTENCODING_UTF8)
		surf = TTF_RenderUTF8_Blended(font, text, RGB_Noname);
	else if(enc == TEXTENCODING_ANSI)
		surf = TTF_RenderText_Blended(font, text, RGB_Noname);
	else if(enc == TEXTENCODING_UNICODE)
		surf = TTF_RenderUNICODE_Blended(font, (Uint16 *)text, RGB_Noname);
	if(!surf)
		sg_log_err("TTF_Render_Blended returns null.");

	return surf;
}

SDL_Surface *image_sdl_clone(const SDL_Surface *src)
{
	assert(src);
	assert(src->w > 0);
	assert(src->h > 0);

	return SDL_ConvertSurface((SDL_Surface *)src, src->format, SDL_SWSURFACE);
}

int image_sdl_copy_to_rgb(SDL_Surface *surf, char **out_buf,
						  size_t *out_size)
{
	assert(surf);
	assert(out_buf);
	assert(out_size);

	*out_size = (size_t)surf->w * surf->h * surf->format->BytesPerPixel;
	*out_buf = malloc(*out_size);
	if(*out_buf == NULL) {
		sg_log_err("out_buf malloc failure.");
		return -1;
	}

	SDL_LockSurface(surf);
	memcpy(*out_buf, surf->pixels, *out_size);
	SDL_UnlockSurface(surf);

	return 0;
}

SDL_Surface *image_sdl_load(const char *path)
{
	SDL_Surface *surf;

	assert(path);
	assert(strlen(path) > 0);

	surf =  IMG_Load(path);
	if(surf == NULL) {
		sg_log_err("Image \"%s\" load error.", path);
		return NULL;
	}

	return surf;
}

SDL_Surface *image_sdl_trim(SDL_Surface *src, uint32_t left,
							uint32_t top, uint32_t right,
							uint32_t bottom)
{
	int ret;
	SDL_Surface *res;
	SDL_Rect rect;
	SDL_BlendMode swap;

	assert(src);
	if(left + right >= src->w || top + bottom >= src->h)
		return NULL;

	if(left == 0 && top == 0 && right == 0 && bottom == 0)
		return image_sdl_clone(src);

	res = image_sdl_create(src->w - left - right,
						   src->h - top - bottom,
						   color_transparent);
	if(!res){
		sg_log_err("Image create failure, %s.", SDL_GetError());
		return NULL;
	}

	rect.x = -left;
	rect.y = -top;
	rect.w = src->w - right;
	rect.h = src->h - bottom;

	ret = SDL_GetSurfaceBlendMode(src, &swap);
	if(ret != 0) {
		sg_log_err("SDL_GetSurfaceBlendMode failure, %s.", SDL_GetError());
		SDL_FreeSurface(res);
		res = NULL;
	}

	SDL_SetSurfaceBlendMode(src, SDL_BLENDMODE_NONE);
	ret = SDL_BlitSurface(src, NULL, res, &rect);
	if(ret != 0) {
		sg_log_err("SDL_BlitSurface failure: %s.", SDL_GetError());
		SDL_FreeSurface(res);
		res = NULL;
	}

	SDL_SetSurfaceBlendMode(src, swap);

	return res;
}

int image_sdl_mask(SDL_Surface *overlay, int32_t overlay_x,
				   int32_t overlay_y, SDL_Surface *dst_image,
				   enum mask_style style)
{
	int ret;
	SDL_Rect overlay_rect;

	assert(overlay);
	assert(dst_image);

	overlay_rect.x = overlay_x;
	overlay_rect.y = overlay_y;
	/* overlay_rect.w = overlay->w + overlay_x; */
	/* overlay_rect.h = overlay->h + overlay_y; */
	if(style == MASKSTYLE_BLEND)
		SDL_SetSurfaceBlendMode(overlay, SDL_BLENDMODE_BLEND);
	else if(style == MASKSTYLE_REPLACE)
		SDL_SetSurfaceBlendMode(overlay, SDL_BLENDMODE_NONE);
	ret = SDL_BlitSurface(overlay, NULL, dst_image, &overlay_rect);
	if(ret != 0)
		sg_log_err("SDL_BlitSurface failure: %s.", SDL_GetError());

	return ret;
}

/* Check for valid PNG format file. */
int image_sdl_check_png(const char *png_path)
{
	int res;
	SDL_RWops *rwop;

	assert(png_path);
	assert(strlen(png_path) > 0);

	if(reg_file_exists(png_path) != 1)
		return -1;

	rwop = SDL_RWFromFile(png_path, "rb");
	if(rwop == NULL) {
		sg_log_err("SDL_RWFromFile(\"%s\") failure, %s.",
				   png_path, SDL_GetError());
		return -1;
	}
	res = IMG_isPNG(rwop) ? 1 : 0;

	SDL_FreeRW(rwop);
	return res;
}

int image_sdl_channel(SDL_Surface *src)
{
	assert(src);

	if(src->format->Amask == 0)
		return 3;
	else
		return 4;
}

void image_sdl_printf_info(const SDL_Surface *src)
{
	if(!src || !src->format)
		return;

	sg_log_inf(
		   "r/g/b/amask:%d/%d/%d/%d, "
           "r/g/b/aloss:%d/%d/%d/%d, "
           "r/g/b/ashift:%d/%d/%d/%d, "
           "flags:%d, format:%d, BitsPerPixel:%d, "
           "pitch:%d, w:%d, h:%d.",
		   src->format->Rmask, src->format->Gmask,
		   src->format->Bmask, src->format->Amask,
		   src->format->Rloss, src->format->Gloss,
		   src->format->Bloss, src->format->Aloss,
		   src->format->Rshift, src->format->Gshift,
		   src->format->Bshift, src->format->Ashift,
		   src->flags, src->format->format,
		   src->format->BitsPerPixel, src->pitch,
		   src->w, src->h);
}

int image_sdl_draw_text(const char *text, enum text_encoding enc,
						struct rgba8888 color,
						int32_t x, int32_t y,
						const char *font_name, uint32_t font_size,
						int font_style, SDL_Surface *draw_dst)
{
	SDL_Surface *surf;

	surf = image_sdl_create_from_text(text, enc, color,
									  font_name, font_size, font_style);
	if(!surf) {
		sg_log_err("image create from text failure.");
		return -1;
	}

	/* Blend text and original image. */
	image_sdl_mask(surf, x, y, draw_dst, MASKSTYLE_BLEND);

	SDL_FreeSurface(surf);
	return 0;
}

int image_sdl_draw_text_magnetic(const char *text, enum text_encoding enc,
								 struct rgba8888 color,
								 int32_t x, int32_t padding_bottom,
								 const char *font_name, uint32_t font_size,
								 int font_style, SDL_Surface *draw_dst)
{
	TTF_Font	*font;
	int			font_height;

	font = font_lib_seek(font_name, font_size);
	if(font == NULL) {
		sg_log_err("font seek (%s, %d) failure.", font_name, font_size);
		return -1;
	}

	font_height = TTF_FontHeight(font);

	return image_sdl_draw_text(text, enc, color, x,
							   draw_dst->h - font_height - padding_bottom,
							   font_name, font_size, font_style, draw_dst);
}

SDL_Surface *image_sdl_set_alpha(SDL_Surface *src, uint8_t alpha)
{
	int ret;
	SDL_Surface *surf;
	Uint8 swap;

	assert(src);

	surf = image_sdl_create((uint32_t)src->w, (uint32_t)src->h, color_transparent);
	if(surf == NULL) {
		sg_log_err("image_create returns null.");
		return NULL;
	}

	SDL_SetSurfaceBlendMode(src, SDL_BLENDMODE_NONE);

	ret = SDL_GetSurfaceAlphaMod(src, &swap);
	if(ret != 0) {
		sg_log_err("SDL_GetSurfaceAlphaMod failure.");
		return NULL;
	}
	SDL_SetSurfaceAlphaMod(src, alpha); /* Set new alpha degree. */
	SDL_BlitSurface(src, NULL, surf, NULL);
	SDL_SetSurfaceAlphaMod(src, swap); /* Reset to original alpha degree. */

	return surf;
}

int image_sdl_erase(SDL_Surface *src)
{
	size_t size;

	assert(src);

	size = (size_t)src->w * src->h *
		   (src->format->BitsPerPixel / 8 /* Bits. */ );
	if(size == 0)
		return -1;

	SDL_LockSurface(src);
	memset(src->pixels, 0, size);
	SDL_UnlockSurface(src);

	return 0;
}

/*
SDL_Surface *image_sdl_resize(SDL_Surface *src, uint32_t new_width,
							uint32_t new_height)
{
	AVFrame *frm_orignal = NULL;
	AVFrame *frm_resize = NULL;
	SDL_Surface *surf = NULL;

	assert(src);
	assert(new_width > 4);
	assert(new_height > 4);

	frm_orignal = image_sdl_surface_to_frame(src);
	if(!frm_orignal) {
		sg_log_err("Surface convert to frame failure.");
		goto end;
	}

	frm_resize = pixel_resize_frm(frm_orignal, new_width, new_height);
	if(!frm_resize) {
		sg_log_err("Pixel resize frame failure.");
		goto end;
	}

	surf = image_sdl_surface_from_frame(frm_resize);
	if(!surf) {
		sg_log_err("Frame convert to surface failure.");
		goto end;
	}

end:
	pixel_free_frm(&frm_orignal);
	pixel_free_frm(&frm_resize);
	return surf;
}*/

int image_sdl_save(SDL_Surface *src, const char *path,
				   enum img_fmt fmt)
{
	int ret = -1;

	assert(src);
	assert(path);
	assert(strlen(path) > 0);
	assert(fmt != IMGFMT_NONE);

	if(fmt == IMGFMT_PNG) {
		ret = IMG_SavePNG(src, path);
		if(ret != 0)
			sg_log_err("Image save error, %s.", SDL_GetError());
	} else
		sg_log_err("Only png is supportted for now.");

	return ret;
}

int image_sdl_pixel_size(const SDL_Surface *src)
{
	assert(src);

	if(!src->format)
		return -1;

	return (src->w * src->h * (src->format->BitsPerPixel / 8 /* Bits. */));
}

AVFrame *image_sdl_surface_to_frame(const SDL_Surface *src)
{
	int size;
	AVFrame *frm = NULL;
	enum AVPixelFormat fmt;

	assert(src);
	assert(src->w > 4);
	assert(src->h > 4);

	switch(src->format->format) {
		case SDL_PIXELFORMAT_RGBA8888:
			fmt = AV_PIX_FMT_ABGR;
			break;
		case SDL_PIXELFORMAT_ARGB8888:
			fmt = AV_PIX_FMT_BGRA;
			break;
		case SDL_PIXELFORMAT_ABGR8888:
			fmt = AV_PIX_FMT_RGBA;
			break;
		case SDL_PIXELFORMAT_BGRA8888:
			fmt = AV_PIX_FMT_ARGB;
			break;
		case SDL_PIXELFORMAT_RGB24:
		case SDL_PIXELFORMAT_RGB888:
			fmt = AV_PIX_FMT_BGR24;
			break;
		case SDL_PIXELFORMAT_BGR888:
		case SDL_PIXELFORMAT_BGR24:
			fmt = AV_PIX_FMT_RGB24;
			break;
		default:
			sg_log_err("Unsupported SDL format %s.", SDL_GetPixelFormatName(src->format->format));
			goto failed;
	}

	frm = pixel_alloc_frm((uint32_t)src->w, (uint32_t)src->h, fmt);
	if(!frm)
		goto failed;

	size = avpicture_get_size(fmt, src->w, src->h);
	if(size <= 0){
		sg_log_err("avpicture_get_size(%d, %d, %d) %d",
				   fmt, src->w, src->h, size);
		goto failed;
	}
	SDL_LockSurface(src);
	memcpy(frm->data[0], src->pixels, (size_t)size);
	SDL_UnlockSurface(src);
	frm->width  = src->w;
	frm->height = src->h;
	frm->format = fmt;
	frm->pts    = 0;

	return frm;

failed:
	pixel_free_frm(&frm);
	return NULL;
}

SDL_Surface *image_sdl_surface_from_frame(const AVFrame *src)
{
	int size;
	AVFrame *frm_ptr;
	AVFrame *swap = NULL;
	SDL_Surface *surf = NULL;
	struct conv_context *cctx = NULL;

	assert(src);
	assert(src->data);
	assert(src->data[0]);
	assert(src->width > 4);
	assert(src->height > 4);
	assert(src->format > AV_PIX_FMT_NONE);
	assert(src->format < AV_PIX_FMT_NB);

	if(src->format == AV_PIX_FMT_RGBA)
		frm_ptr = (AVFrame *)src;
	else {
		cctx = pixel_conv_get_context();
		if(!cctx)
			goto cleanup;
		pixel_conv_set_dst_fmt(cctx, AV_PIX_FMT_RGBA);
		swap = pixel_conv_alloc(cctx, src);
		if(!swap)
			goto cleanup;
		frm_ptr = swap;
	}

	size = avpicture_get_size((enum AVPixelFormat)frm_ptr->format,
							  frm_ptr->width, frm_ptr->height);
	surf = image_sdl_create((uint32_t)frm_ptr->width, (uint32_t)frm_ptr->height,
							color_transparent);
	if(!surf)
		goto cleanup;
	SDL_LockSurface(surf);
	memcpy(surf->pixels, frm_ptr->data[0], (size_t)size);
	SDL_UnlockSurface(surf);

cleanup:
	pixel_conv_free_context(&cctx);
	pixel_free_frm(&swap);

	return surf;
}

void image_sdl_uninit(void)
{
	IMG_Quit();
	SDL_Quit(); /* 864 bytes memory leaks. */
	font_lib_uninit();
	sg_log_inf("Image uninit.");
}