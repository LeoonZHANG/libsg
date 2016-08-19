/*
 * base64.h
 * Author: wangwei.
 * Base64 encoder and decoder.
 */

#ifndef LIBSG_BASE64_H
#define LIBSG_BASE64_H

#include <stdlib.h> /* size_t */
#include "..//str/vlstr.h"
#include "../util/vlbuf.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct sg_base64_enc;
struct sg_base64_dec;

sg_vlstr *
sg_base64_easy_enc_buf(void *data, size_t size);

struct sg_vlbuf *
sg_base64_easy_dec_buf(const char *base64_str);

struct sg_base64_enc *
sg_base64_enc_start(void);

void
sg_base64_enc_update(struct sg_base64_enc *, void *data, size_t size);

int
sg_base64_enc_finish(struct sg_base64_enc *, sg_vlstr *);

struct sg_base64_dec *
sg_base64_dec_start(void);

void
sg_base64_dec_update(struct sg_base64_dec *, void *data, size_t size);

int
sg_base64_dec_finish(struct sg_base64_dec *, sg_vlstr *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_BASE64_H */
