/**
 * zip.h
 * 基于zlib、zip和snappy的压缩库
 * zip 基于 https://github.com/kjk/zip (unix-like) 和 https://github.com/madler/zip (windows port) 和google snappy压缩库
 * 如果压缩库支持不同的压缩算法、压缩比,请把这样的参数暴露出来
 * zlib的但使用可以灿开zlib模块(参考完毕就可以删掉它了)
 */

#ifndef LIBSG_ZIP_H
#define LIBSG_ZIP_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum sg_zip_type {
    SGZIPTYPE_ZLIB = 0,
    SGZIPTYPE_PIGZ = 1,
    SGZIPTYPE_SNAPPY = 2
};

enum sg_zip_mode {
    SGZIPMODE_COMPRESS   = 0,
    SGZIPMODE_UNCOMPRESS = 1
};

typedef struct sg_zip_ctx_in sg_zip_t;



/* easy APIs */

typedef void (*sg_zip_on_data_func_t)(const void *data, size_t size, void *user_data);

int sg_zip_buf(enum sg_zip_mode, const void *buf, size_t len, sg_vsbuf *out_buf);

int sg_zip_str(enum sg_zip_mode, const char *str, size_t len, sg_vsstr *out_str);

int sg_zip_file(enum sg_zip_mode, const char *path, const char *out_path);




/* low level APIs */

sg_zip_t *sg_zip_start(sg_zip_on_data_func_t cb);

void sg_zip_set_user_data(sg_zip_t *self, void *user_data);

int sg_zip_put(sg_zip_t *self, void *data, uint64_t size);

int sg_zip_finish(sg_zip_t *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_PIGZ_H */
