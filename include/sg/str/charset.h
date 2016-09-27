/**
 * charset.h
 * Char encoding detective and transcoding.
 */

#ifndef LIBSG_CHARSET_H
#define LIBSG_CHARSET_H

#include <sg/sg.h>
#include <sg/container/vsbuf.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
const char *sg_charset_detect(const char *str, const uint64_t str_len);
*/

/* alloc space for out_buf befroe useing this function */
bool sg_charset_conv(const char *from_charset, const char *to_charset,
        const void *from_str, const uint64_t from_str_len,
        const void *out_buf, const uint64_t out_buf_size, uint64_t *real_out_str_len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_CHARSET_H */
