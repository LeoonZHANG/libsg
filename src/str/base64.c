/**
 * base64.c
 * Base64 encoder and decoder.
 */

#include <string.h>
#include <sg/sg.h>
#include "../../3rdparty/b64.c/b64.h"
#include <sg/str/base64.h>

size_t _base64_calc_enc_len(size_t data_len);

/* decoded data size equals dec_size, or 1~2 byte(s) smaller than dec_size */
size_t _base64_calc_dec_len(size_t b64_str_len);

size_t _base64_calc_enc_len(size_t data_len)
{
    if (data_len == 0)
        return 0;

    if (data_len % 3 != 0)
        data_len += 3 - (data_len % 3);

    return (data_len / 3) * 4;
}

size_t _base64_estimate_dec_len(size_t base64_str_len)
{
    SG_ASSERT_MSG(base64_str_len % 4 == 0, "Error base64 string length %lu.", base64_str_len);

    return (base64_str_len / 4) * 3;
}

int sg_base64_enc(const void *bin_buf, size_t bin_buf_len, sg_vsstr_t *b64_str)
{
    size_t b64_size;
    char *b64_buf;

    SG_ASSERT(bin_buf);
    SG_ASSERT(bin_buf_len > 0);
    SG_ASSERT(b64_str);

    b64_size = _base64_calc_enc_len(bin_buf_len) + 1 /* Terminator */;
    b64_buf = b64_encode((const unsigned char *)bin_buf, bin_buf_len);

    sg_vsstr_cpy(b64_str, b64_buf);
    free(b64_buf);
    return 0;
}

int sg_base64_dec(const char *b64_str, size_t b64_str_len, sg_vsbuf_t *bin_buf)
{
    unsigned char *dec_buf;
    size_t dec_size;

    SG_ASSERT(b64_str);
    SG_ASSERT(b64_str_len > 0);
    SG_ASSERT(bin_buf);

    dec_buf = (unsigned char *)malloc(_base64_estimate_dec_len(b64_str_len));
    if (!dec_buf)
        return 0;

    dec_buf = b64_decode_ex(b64_str, b64_str_len, &dec_size);
    sg_vsbuf_insert(bin_buf, (const char*)(dec_buf), dec_size);
    free(dec_buf);
    return 0;
}