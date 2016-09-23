/**
 * base64.c
 * Test for util module base64.
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <sg/util/log.h>
#include <sg/str/base64.h>
#include <sg/str/vsstr.h>
#include <gtest/gtest.h>

TEST(test_sg_base64, enc_dec)
{
    const uint8_t bin_buf[20] = {
        0x03, 0x14, 0x0e, 0x00, 0x15, 0x84, 0xa3, 0xf3,
        0xc8, 0xd5, 0x14, 0x0e, 0x00, 0x15, 0xac, 0xcb,
        0x14, 0x0e, 0x00, 0x15
    };
    size_t bin_size = sizeof(bin_buf);
    sg_vsstr_t *b64_str;
    sg_vsbuf_t *raw_bin;

    b64_str = sg_vsstr_alloc();
    sg_base64_enc((void *)bin_buf, bin_size, b64_str);

    raw_bin = sg_vsbuf_create();
    sg_base64_dec(sg_vsstr_raw(b64_str), sg_vsstr_len(b64_str), raw_bin);

    ASSERT_TRUE(raw_bin->size == bin_size && memcmp(raw_bin->mem, bin_buf, bin_size) == 0);

    if (b64_str)
        sg_vsstr_free(&b64_str);
    if (raw_bin)
        sg_vsbuf_destroy(raw_bin);
}
