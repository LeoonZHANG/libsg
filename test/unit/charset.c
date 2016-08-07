/*
 * charset.c
 * Author: wangwei.
 * Test for util module charset.
 */

#include <stddef.h>
#include <string.h>
#include "../util/log.h"
#include "../util/charset.h"

#define TEST_CHARSET_IOLEN 1000

int main(void)
{
    char   in[TEST_CHARSET_IOLEN];
    size_t in_len = TEST_CHARSET_IOLEN;
    char   out[TEST_CHARSET_IOLEN];
    size_t out_len = TEST_CHARSET_IOLEN;

    memset(in, 0, TEST_CHARSET_IOLEN);
    strcpy(in, "疯狂与偏执，最终将在人类文明的内部异化出怎样的力量？冷酷的星空将如何拷问心中道德？");
    sg_log_inf("Source GBK string:%s.", in);
    charset_conv("GBK", "utf-8", in, in_len, out, out_len);
    sg_log_inf("Convert utf-8 string:%s.", out);
    charset_conv("utf-8", "GBK", out, in_len, in, out_len);
    sg_log_inf("Convert GBK string:%s.", in);

    return 0;
}