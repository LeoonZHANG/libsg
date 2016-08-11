/*
 * base64.c
 * Author: wangwei.
 * Base64 encoder and decoder.
 */

#include <string.h>
#if 0
# include "../3rdparty/libb64-1.2.1/include/b64/cdecode.h"
# include "../3rdparty/libb64-1.2.1/include/b64/cencode.h"
#endif
#include "../../3rdparty/b64.c/b64.h"
#include "../../include/str/vlstr.h"
#include "../../include/str/base64.h"
#include "../../include/util/assert.h"
#include "../../include/util/log.h"

size_t sg_base64_calc_enc_len(size_t data_len);

/* decoded data size equals dec_size, or 1~2 byte(s) smaller than dec_size */
size_t sg_base64_calc_dec_len(size_t base64_str_len);

size_t sg_base64_calc_enc_len(size_t data_len)
{
    if (data_len == 0)
        return 0;

    if (data_len % 3 != 0)
        data_len += 3 - (data_len % 3);

    return (data_len / 3) * 4;
}

size_t sg_base64_estimate_dec_len(size_t base64_str_len)
{
    if (base64_str_len % 4 != 0) {
        sg_log_err("Error base64 string length %lu.", base64_str_len);
        sg_assert(0);
    }

    return (base64_str_len / 4) * 3;
}

#if 0
sg_vlstr *sg_base64_easy_enc_buf(void *data, size_t size)
{
    int cnt = 0; /* the number of bytes encoded by a single call */
    base64_encodestate s; /* encoder state */
    size_t b64_size;
    sg_vlstr *b64_str;
    char *b64_buf;

    sg_assert(data);
    sg_assert(size > 0);

    b64_size = sg_base64_calc_enc_len(size) + 1 /* Terminator */;
    sg_log_inf("Calc base64 len %lu", b64_size);
    b64_buf = (char *)malloc(b64_size);
    if (!b64_buf)
        return NULL;
    memset(b64_buf, 1, b64_size);

    base64_init_encodestate(&s);
    cnt += base64_encode_block((const char *)data, (int)size, b64_buf, &s);
    sg_log_inf("cnt %d", cnt);
    cnt += base64_encode_blockend(b64_buf + cnt, &s);
    sg_log_inf("cnt %d", cnt);
    b64_buf[cnt - 1] = 0; /* terminator, it's different from demo! */

    b64_str = sg_vlstrndup(b64_buf, b64_size - 1);
    sg_log_inf("b64_str len %lu.", (size_t)sg_vlstrlen(b64_str));
    free(b64_buf);
    return b64_str;
}

struct sg_vlbuf *sg_base64_easy_dec_buf(const char *base64_str)
{
    int cnt = 0; /* the number of bytes decoded by a single call */
    base64_decodestate s; /* decoder state */
    struct sg_vlbuf *buf;
    void *dec_buf;
    size_t dec_size;

    sg_assert(base64_str);
    sg_assert(strlen(base64_str) > 0);

    dec_buf = malloc(sg_base64_estimate_dec_len(strlen(base64_str)));
    if (!dec_buf)
        return NULL;
    buf = sg_vlbuf_create();
    if (!buf) {
        free(dec_buf);
        return NULL;
    }

    base64_init_decodestate(&s);
    cnt += base64_decode_block(base64_str, strlen(base64_str), (char *)dec_buf, &s);
    dec_size = cnt;
    sg_vlbuf_insert(buf, dec_buf, dec_size);

    return buf;
}
#endif

sg_vlstr *sg_base64_easy_enc_buf(void *data, size_t size)
{
    size_t b64_size;
    sg_vlstr *b64_str;
    char *b64_buf;

    sg_assert(data);
    sg_assert(size > 0);

    b64_size = sg_base64_calc_enc_len(size) + 1 /* Terminator */;
    b64_buf = b64_encode((const unsigned char *)data, size);

    b64_str = sg_vlstralloc2(b64_buf);
    free(b64_buf);
    return b64_str;
}

struct sg_vlbuf *sg_base64_easy_dec_buf(const char *base64_str)
{
    struct sg_vlbuf *buf;
    unsigned char *dec_buf;
    size_t dec_size;

    sg_assert(base64_str);
    sg_assert(strlen(base64_str) > 0);

    dec_buf = (unsigned char *)malloc(sg_base64_estimate_dec_len(strlen(base64_str)));
    if (!dec_buf)
        return NULL;
    buf = sg_vlstralloc();
    if (!buf) {
        free(dec_buf);
        return NULL;
    }

    dec_buf = b64_decode_ex(base64_str, strlen(base64_str), &dec_size);
    sg_vlstrncat(buf, (const char*)(dec_buf), dec_size);
    free(dec_buf);
    return buf;
}
