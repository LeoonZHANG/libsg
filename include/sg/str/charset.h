/*
 * charset.h
 * Author: wangwei.
 * Char encoding detective and transcoding.
 */

#ifndef LIBSG_CHARSET_H
#define LIBSG_CHARSET_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// BIG5 -> GBK

/*
vsstr *charset_detect(const char *src);
 */

int sg_charset_conv(const char *from_charset, const char *to_charset, const char *from_str, size_t from_len,
                    char *out_str, size_t out_max_len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_CHARSET_H */
