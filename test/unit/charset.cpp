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
    char   in[TEST_CHARSET_IOLEN];
    size_t in_len = TEST_CHARSET_IOLEN;
    char   out[TEST_CHARSET_IOLEN];
    size_t out_len = TEST_CHARSET_IOLEN;
    const char* text = "疯狂与偏执，最终将在人类文明的内部异化出怎样的力量？冷酷的星空将如何拷问心中道德？";

    memset(in, 0, TEST_CHARSET_IOLEN);
    strcpy(in, text);
/* Visual C++ 2005 and latter will encode multibyte string as local multibyte encodding instead of utf-8 */
#if defined (_MSC_VER) && _MSC_VER > 1310
	sg_log_inf("Source GBK string:%s.", in);
	sg_charset_conv("GBK", "utf-8", in, in_len, out, out_len);
	sg_log_inf("Convert to utf-8 string:%s.", out);
	sg_charset_conv("utf-8", "GBK", out, in_len, in, out_len);
	sg_log_inf("Convert to GBK string:%s.", in);
#else
    sg_log_inf("Source utf-8 string:%s.", in);
    sg_charset_conv("utf-8", "GBK", in, in_len, out, out_len);
    sg_log_inf("Convert to GBK string:%s.", out);
    sg_charset_conv("GBK", "utf-8", out, in_len, in, out_len);
    sg_log_inf("Convert to utf-8 string:%s.", in);
#endif
    ASSERT_STREQ(in, text);
}
