/*
 * font.h
 * Author: wangwei.
 * TTF font management library, based on SDL_ttf.
 */

#ifndef IMGUTIL_FONT_H
#define IMGUTIL_FONT_H

#include "../util/vlstr_list.h"

/* Actually, it is a TTF_Font pointer. */
typedef void mw_font;

/* Font style, could use them together with OR (|) operation. */
enum font_style {
    FONTSTYLE_NORMAL        = 0x00, /* TTF_STYLE_NORMAL. */
    FONTSTYLE_BOLD          = 0x01, /* TTF_STYLE_BOLD. */
    FONTSTYLE_ITALIC        = 0x02, /* TTF_STYLE_ITALIC. */
    FONTSTYLE_UNDERLINE     = 0x04, /* TTF_STYLE_UNDERLINE. */
    FONTSTYLE_STRIKETHROUGH = 0x08  /* TTF_STYLE_STRIKETHROUGH. */
};

/* Initialize font library, load all fonts from system fonts directory. */
int font_lib_init(void);

/* Free font library.
   Do not call it before font_lib_init. */
void font_lib_uninit(void);

/* Add .TTF file to font library which isn't in system fonts directory.
   After this, you can seek it from the library by font name. */
int font_lib_register(const char *font_path);

/* Get font pointer by font name (not filename) and size.
   Should register it before. */
mw_font *font_lib_seek(const char *font_name, uint32_t font_size);

/* Set font style. */
void font_set_style(mw_font *font, int style);

#endif //IMGUTIL_FONT_H