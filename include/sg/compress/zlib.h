/**
 * zlib.h
 * 将被删除, 留在这里是因为两个接口可以为zip模块提供参考
 */

#ifndef LIBSG_ZLIB_H
#define LIBSG_ZLIB_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum sg_zip_fmt {
    SGZIPFMT_GZIP,
    SGZIPFMT_ZIP
};

int sg_zip_compress(char *in, int in_size, char **dest, int *dest_size);

int sg_zip_uncompress(char *in, int in_size, char **dest, int *dest_size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_ZLIB_H */
