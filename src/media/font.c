/*
 * font.c
 * Author: wangwei.
 * TTF font management library, based on SDL_ttf.
 */

#include <stdint.h>
#include <stdint-gcc.h>
#include "../util/os.h"

#ifdef OS_WIN
#include <Wingdi.h> ///< EnumFonts.
#endif

#ifdef OS_LNX
#include "../util/shell.h"
#endif

#include "SDL2/SDL_ttf.h"
#include "../util/log.h"
#include "../util/md5.h"
#include "../util/file.h"
#include "../util/list.h"
#include "../util/mutex.h"
#include "../util/macro.h"
#include "../util/assert.h"
#include "font.h"

#define FONT_SIZE_DEFAULT 40
#define SAFE_TTF_CLOSEFONT(P) if(P) { TTF_CloseFont(P); (P) = NULL; }
#define FONT_SIZE_MIN 1
#define FONT_SIZE_MAX 200

struct font_ptr{
    TTF_Font    *ttf;
    uint32_t    size;
};

struct font_inf{
    char           *name;
    char           *path;
    struct mw_list *ptr_list;
};

static struct mw_list  *font_list = NULL;
static struct mw_mutex font_list_mtx;

int  load_system_fonts(void);
int  font_list_load_file(const char *path, uint32_t font_size);
int  ptr_list_add(struct font_inf *fi, struct font_ptr *fp);
void font_search_callback(enum shell_event ev, const char *data, void *context);
struct font_ptr *ptr_list_seek_size(struct font_inf *fi, uint32_t size);
struct font_inf *font_list_seek_name(const char *font_name);
struct font_inf *font_list_seek_path(const char *font_path);
TTF_Font *font_list_seek_name_size(const char *font_name, uint32_t font_size);
TTF_Font *font_list_seek_path_size(const char *font_path, uint32_t font_size);
vlstr_list *search_system_fonts(void);
#if defined(OS_LNX) || defined(OS_OSX)
void font_search_callback(enum shell_event ev, const char *data, void *context);
#endif
#if defined(OS_WIN)
void font_search_callback(enum dir_seek_event evt, const char *path,
                          struct stat *s, void *context);
#endif


int font_lib_init(void)
{
    if (TTF_Init() != 0) {
        sg_log_err("TTF_Init failure, %s\n", TTF_GetError());
        return -1;
    }

    mutex_create(&font_list_mtx);
    font_list = mw_list_init();
    if(font_list == NULL)
        return -1;

    if(load_system_fonts() != 0) {
        mutex_destroy(&font_list_mtx);
        TTF_Quit();
        return -1;
    }

    if(!font_lib_seek("SimHei", FONT_SIZE_DEFAULT)){
        sg_log(LL_FATAL, "SimHei font not found.");
        return -1;
    }

    return 0;
}

void font_lib_uninit(void)
{
    struct mw_item  *c;
    struct mw_item  *cu;
    struct font_ptr *fp;
    struct font_inf *fi;

    mutex_lock(&font_list_mtx);
    if(font_list)
        cu = mw_list_begin(font_list);
    else
        cu = NULL;
    while (cu) {
        fi = (struct font_inf *)cu->data;
        if(fi == NULL){
            cu = cu->next;
            continue;
        }
        SAFE_FREE(fi->path);
        SAFE_FREE(fi->name);
        if(fi->ptr_list) {
            c = mw_list_begin(fi->ptr_list);
            while (c) {
                fp = (struct font_ptr *)c->data;
                if(fp == NULL) {
                    c = c->next;
                    continue;
                }
                SAFE_TTF_CLOSEFONT(fp->ttf);
                c = c->next;
            }
        }
        cu = cu->next;
    }
    mutex_unlock(&font_list_mtx);

    mutex_destroy(&font_list_mtx);
    SAFE_MW_LIST_UNINIT(font_list);

    TTF_Quit();
}

int font_lib_register(const char *font_path)
{
    return font_list_load_file(font_path, FONT_SIZE_DEFAULT);
}

mw_font *font_lib_seek(const char *font_name, uint32_t font_size)
{
    TTF_Font *res;
    struct font_inf *fi;

    assert(font_name);
    assert(strlen(font_name) > 0);
    assert(font_size > 0);

    res = font_list_seek_name_size(font_name, font_size);
    if(res)
        return res;

    fi = font_list_seek_name(font_name);
    if(fi && fi->path && (strlen(fi->path) > 0)){
        font_list_load_file(fi->path, font_size);
        return font_list_seek_name_size(font_name, font_size);
    }

    return NULL;
}

void font_set_style(mw_font *font, int style)
{
    assert(font);
    assert(style >= 0);

    TTF_SetFontStyle((TTF_Font *)font, style);
}

/* Insert font ptr to ptr list of font info. */
int ptr_list_add(struct font_inf *fi, struct font_ptr *fp)
{
    vlstr *id;
    struct mw_item *swap;

    assert(fi);
    assert(fi->ptr_list);
    assert(fp);

    id = vlstrfmt("%d", fp->size);
    if(id == NULL)
        return -1;

    mutex_lock(&font_list_mtx);
    swap = mw_list_add_item(fi->ptr_list, ITEMDATATYPE_REOPEN, (void *)fp,
                            sizeof(struct font_ptr), 0, vlstrraw(id));
    mutex_unlock(&font_list_mtx);

    vlstrfree(&id);
    return swap ? 0 : -1;
}

/* Load and insert font by font path and font size. */
int font_list_load_file(const char *font_path, uint32_t font_size)
{
    int ret;
    TTF_Font *ttf;
    struct font_ptr fp_new;
    struct font_inf fi_new;
    struct font_inf *fi_seek;

    assert(font_path);
    assert(strlen(font_path) > 0);
    assert(font_size >= FONT_SIZE_MIN);
    assert(font_size <= FONT_SIZE_MAX);

    if(font_list_seek_path_size(font_path, font_size))
        return 0; /* do nothing */

    ttf = TTF_OpenFont(font_path, font_size);
    if (ttf == NULL) {
        sg_log_err("TTF font %s open failure, %s.", font_path, TTF_GetError());
        return -1;
    }
    fp_new.ttf  = ttf;
    fp_new.size = font_size;

    fi_seek = font_list_seek_path(font_path);
    if(!fi_seek) {
        fi_new.name = strdup(TTF_FontFaceFamilyName(ttf));
        fi_new.path = strdup(font_path);
        fi_new.ptr_list = mw_list_init();
        if (!fi_new.name || !fi_new.path || !fi_new.ptr_list) {
            SAFE_FREE(fi_new.name);
            SAFE_FREE(fi_new.path);
            SAFE_MW_LIST_UNINIT(fi_new.ptr_list);
            TTF_CloseFont(ttf);
            sg_log_err("Font info strdup failure.");
            return -1;
        }

        mutex_lock(&font_list_mtx);
        mw_list_add_item(font_list, ITEMDATATYPE_REOPEN,
                         (void *)&fi_new, sizeof(struct font_inf),
                         0, (char *)md5_str(font_path, MD5FMT_STR).buf);
        mutex_unlock(&font_list_mtx);
    }

    fi_seek = font_list_seek_path(font_path);
    if(!fi_seek){
        sg_log_err("Font list insert font info failure.");
        return -1;
    }

    ret = ptr_list_add(fi_seek, &fp_new);
    if(ret != 0)
        sg_log_err("Font info add font error.");
    else
        sg_log_dbg("Font loaded, name:%s, size:%d, path:%s.",
                   TTF_FontFaceFamilyName(ttf), font_size, font_path);

    return 0;
}

/* Seek font ptr from ptr list by font size. */
struct font_ptr *ptr_list_seek_size(struct font_inf *fi, uint32_t size)
{
    vlstr *seek;
    struct mw_item *swap;

    seek = vlstrfmt("%d", size);
    if(seek == NULL)
        return NULL;
    swap = mw_list_seek_id(fi->ptr_list, vlstrraw(seek));

    vlstrfree(&seek);

    return swap ? (struct font_ptr *)swap->data : NULL;
}

/* Seek font info from font list by font path. */
struct font_inf *font_list_seek_path(const char *font_path)
{
    struct mw_item *it;
    struct font_inf *fi = NULL;

    if(!font_path || (strlen(font_path) == 0))
        return NULL;

    mutex_lock(&font_list_mtx);
    if(!font_list){
        mutex_unlock(&font_list_mtx);
        return NULL;
    }
    it = mw_list_seek_id(font_list,
                         (char *)md5_str(font_path, MD5FMT_STR).buf);
    if(it)
        fi = (struct font_inf *)it->data;
    mutex_unlock(&font_list_mtx);

    return fi;
}

/* Seek font info from font list by font name. */
struct font_inf *font_list_seek_name(const char *font_name)
{
    struct mw_item *cur;
    struct font_inf *fi;

    if(!font_name || (strlen(font_name) == 0))
        return NULL;

    mutex_lock(&font_list_mtx);
    if(font_list)
        cur = mw_list_begin(font_list);
    else
        cur = NULL;
    while(cur) {
        fi = (struct font_inf *)cur->data;
        if(fi == NULL || fi->name == NULL || fi->ptr_list == NULL) {
            cur = cur->next;
            continue;
        }
        if(strcmp(fi->name, font_name) == 0){
            mutex_unlock(&font_list_mtx);
            return fi;
        }
        cur = cur->next;
    }
    mutex_unlock(&font_list_mtx);

    return NULL;
}

/* Seek TTF_Font from font list by font name and font size. */
TTF_Font *font_list_seek_name_size(const char *font_name, uint32_t font_size)
{
    struct font_inf *fi;
    struct font_ptr *fp;

    assert(font_name);
    assert(font_size >= FONT_SIZE_MIN);
    assert(font_size <= FONT_SIZE_MAX);
    assert(strlen(font_name) > 0);

    fi = font_list_seek_name(font_name);
    if(!fi)
        return NULL;

    fp = ptr_list_seek_size(fi, font_size);
    if (!fp)
        return NULL;

    return fp->ttf;
}

/* Seek TTF_Font from font list by font path and font size. */
TTF_Font *font_list_seek_path_size(const char *font_path, uint32_t font_size)
{
    struct font_inf *fi;
    struct font_ptr *fp;

    assert(font_path);
    assert(font_size >= FONT_SIZE_MIN);
    assert(font_size <= FONT_SIZE_MAX);
    assert(strlen(font_path) > 0);

    fi = font_list_seek_path(font_path);
    if(!fi)
        return NULL;

    fp = ptr_list_seek_size(fi, font_size);
    if (!fp)
        return NULL;

    return fp->ttf;
}

#if defined(OS_LNX) || defined(OS_OSX)

/* Callback for search system fonts. */
void font_search_callback(enum shell_event ev, const char *data, void *context)
{
    vlstr_list *vl = (vlstr_list *)context;
    char *str = (char *)data;
    char *end;

    if(!vl || !str || strlen(str) == 0)
        return;

    if(ev == SHELLEVENT_DATA) {
        end = strchr(data, ':');
        if(end == NULL || end <= data)
            return;
        vlstr_list_push2(vl, str, end - data);
    }
}

/* Search posix system fonts. */
vlstr_list *search_system_fonts(void)
{
    char       *s;
    vlstr      *ext;
    size_t     size;
    vlstr_list *vl = NULL;

    vl = vlstr_list_alloc();
    if(!vl)
        return NULL;

    /* Load zh-cn fonts only, fc-list :lang=zh file */
    shell_exec("fc-list : file", font_search_callback, (void *)vl);

    return vl;
}

#endif

#if defined(OS_WIN)

/* Callback for search system fonts. */
void font_search_callback(enum dir_seek_event evt, const char *path,
                          struct stat *s, void *context)
{
    vlstr *ext;
    vlstr_list *vl = (vlstr_list *)context;

    if(!vl || !path || strlen(path) == 0)
        return;

    if(evt != DIRSEEKEVENT_FOUND)
        return;

    ext = file_get_ext(path, 1);
    if(strcmp(vlstrraw(ext), ".TTF") == 0)
        vlstr_list_push(vl, path);

    vlstrfree(&ext);
}

/* Search windows system fonts. */
vlstr_list *search_system_fonts(void)
{
    char font_dir[MAX_PATH];
    vlstr_list *vl = NULL;

    vl = vlstr_list_alloc();
    if(!vl)
        return NULL;

    GetSystemDirectory(font_dir, MAX_PATH);
    if(font_dir[strlen(font_dir)] != '\\' && font_dir[strlen(font_dir)] != '/')
        strcat(font_dir, "/");
    strcat(font_dir, "fonts/");

    dir_seek(font_dir, 1, font_seek_callback, (void *)vl);

    return vl;
}

#endif

/* Load TTF fonts and insert them to list. */
int load_system_fonts(void)
{
    char       *s;
    vlstr      *ext;
    size_t     size, i;
    vlstr_list *vl = NULL;

    vl = search_system_fonts();
    if(!vl){
        sg_log_err("Fonts search failure.");
        return -1;
    }

    size = vlstr_list_size(vl);
    if(size == 0)
        sg_log_err("No font was loaded.");
    for(i = 0; i < size; i++) {
        s = vlstr_list_get(vl, (int)i);
        if(strlen(s) == 0)
            continue;
        ext = file_get_ext(s, 1);
        if(!ext || vlstrlen(ext) == 0)
            continue;
        if(strcmp(vlstrraw(ext), ".TTF") == 0)
            font_list_load_file(s, FONT_SIZE_DEFAULT);
        vlstrfree(&ext);
    }

    vlstr_list_free(&vl);
    return 0;
}