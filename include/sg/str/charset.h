/**
 * charset.h
 * Char encoding detective and transcoding.
 */

#ifndef LIBSG_CHARSET_H
#define LIBSG_CHARSET_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
vsstr *charset_detect(const char *src);
 */

int sg_charset_conv(const char *from_charset, const char *to_charset,
        const char *from_str, sg_vsstr_t *out_str);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_CHARSET_H */
