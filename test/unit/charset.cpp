/**
 * charset.c
 * Test for util module charset.
 */

#include <stddef.h>
#include <string.h>
#include <sg/util/log.h>
#include <sg/str/charset.h>
#include <gtest/gtest.h>

#define TEST_CHARSET_IOLEN 1000

TEST(test_sg_charset, conv) {
    const char *in  = "疯狂与偏执，最终将在人类文明的内部异化出怎样的力量？冷酷的星空将如何拷问心中道德？";
    char out1[TEST_CHARSET_IOLEN] = {0}, out2[TEST_CHARSET_IOLEN] = {0};
    uint64_t out1_str_len = 0, out2_str_len = 0;

/* Visual C++ 2005 and latter will encode multibyte string as local multibyte encodding instead of utf-8 */
#if defined (_MSC_VER) && _MSC_VER > 1310
	sg_log_inf("Source GBK string:%s.", in);
	sg_charset_conv("GBK", "utf-8", in, strlen(in), out1, TEST_CHARSET_IOLEN, &out1_str_len);
	sg_log_inf("Convert to utf-8 string:%s.", out1);
	sg_charset_conv("utf-8", "GBK", out1, out1_str_len, out2, TEST_CHARSET_IOLEN, &out2_str_len);
	sg_log_inf("Convert to GBK string:%s.", out2);
#else
    sg_log_inf("Source utf-8 string:%s.", in);
    sg_charset_conv("utf-8", "GBK", in, strlen(in), out1, TEST_CHARSET_IOLEN, &out1_str_len);
    sg_log_inf("Convert to GBK string:%s.", out1);
    sg_charset_conv("GBK", "utf-8", out1, out1_str_len, out2, TEST_CHARSET_IOLEN, &out2_str_len);
    sg_log_inf("Convert to utf-8 string:%s.", out2);
#endif
    ASSERT_STREQ(out2, in);
}
