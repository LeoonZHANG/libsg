/*
 * base64.h
 * Author: wangwei.
 * Base64 encoder and decoder.
 */

#ifndef LIBSG_BASE64_H
#define LIBSG_BASE64_H

#include <stdlib.h> /* size_t */
#include "../str/vlstr.h"
#include "../util/vlbuf.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int sg_base64_enc(const void *bin_buf, size_t bin_buf_len, sg_vlstr_t *b64_str);

int sg_base64_dec(const char *b64_str, size_t b64_str_len, sg_vlbuf_t *bin_buf);

/*
sg_vlstr *sg_base64_enc(const void *data, size_t size);

struct sg_vlbuf *sg_base64_dec(const char *b64_str, size_t b64_str_len);*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_BASE64_H */
