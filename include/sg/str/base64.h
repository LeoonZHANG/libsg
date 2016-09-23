/*
 * base64.h
 * Author: wangwei.
 * Base64 encoder and decoder.
 */

#ifndef LIBSG_BASE64_H
#define LIBSG_BASE64_H

#include <sg/sg.h>
#include <sg/str/vsstr.h>
#include <sg/container/vsbuf.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int sg_base64_enc(const void *bin_buf, size_t bin_buf_len, sg_vsstr_t *b64_str);

int sg_base64_dec(const char *b64_str, size_t b64_str_len, sg_vsbuf_t *bin_buf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_BASE64_H */
