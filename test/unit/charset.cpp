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
    sg_vsstr_t *dst1 = sg_vsstr_alloc();
    sg_vsstr_t *dst2 = sg_vsstr_alloc();
    const char *src  = "疯狂与偏执，最终将在人类文明的内部异化出怎样的力量？冷酷的星空将如何拷问心中道德？";

/* Visual C++ 2005 and latter will encode multibyte string as local multibyte encodding instead of utf-8 */
#if defined (_MSC_VER) && _MSC_VER > 1310
	sg_log_inf("Source GBK string:%s.", src);
	sg_charset_conv("GBK", "utf-8", src, dst1);
	sg_log_inf("Convert to utf-8 string:%s.", sg_vsstr_raw(dst1));
	sg_charset_conv("utf-8", "GBK", sg_vsstr_raw(dst1), dst2);
	sg_log_inf("Convert to GBK string:%s.", sg_vsstr_raw(dst2));
#else
    sg_log_inf("Source utf-8 string:%s.", src);
    sg_charset_conv("utf-8", "GBK", src, dst1);
    sg_log_inf("Convert to GBK string:%s.", sg_vsstr_raw(dst1));
    sg_charset_conv("GBK", "utf-8", sg_vsstr_raw(dst1), dst2);
    sg_log_inf("Convert to utf-8 string:%s.", sg_vsstr_raw(dst2));
#endif
    ASSERT_STREQ(sg_vsstr_raw(dst2), src);
    sg_vsstr_free(&dst1);
    sg_vsstr_free(&dst2);
}
