/*
 * base64.c
 * Author: wangwei.
 * Test for util module base64.
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <sg/util/log.h>
#include <sg/sys/file.h>
#include <sg/str/base64.h>
#include <sg/str/vlstr.h>

int main(void)
{
    uint8_t *bin_buf = NULL;
    size_t bin_size;
    sg_vlstr_t *b64_str;
    sg_vlbuf_t *raw_bin;

    const char *s = "hello world!!!!";
    sg_file_to_buf("/home/metalwood/file1.mp3", &bin_buf, &bin_size);
    b64_str = sg_vlstralloc();
    sg_base64_enc((void *)bin_buf, bin_size, b64_str);
    sg_log_inf("base64 len:%"PRIu64".", sg_vlstrlen(b64_str));
    sg_file_overwrite("/home/metalwood/file-base64.txt", (uint8_t *)sg_vlstrraw(b64_str), sg_vlstrlen(b64_str));
    /*if (bin_buf)
        free(bin_buf); *//* double free? */
    sg_log_inf("Raw size %lu, base64 string lenght %"PRIu64".", bin_size, sg_vlstrlen(b64_str));

    raw_bin = sg_vlbuf_create();
    sg_base64_dec(b64_str, sg_vlstrlen(b64_str), raw_bin);
    sg_file_overwrite("/home/metalwood/file2.mp3", (uint8_t *)raw_bin->mem, raw_bin->size);

    sg_log_inf("Raw size %lu, base64 string lenght %"PRIu64", decoded size %lu.", bin_size, sg_vlstrlen(b64_str), raw_bin->size);

    if (bin_buf)
        free(bin_buf);
    if (b64_str)
        sg_vlstrfree(&b64_str);
    if (raw_bin)
        sg_vlbuf_destroy(raw_bin);
    return 0;
}
