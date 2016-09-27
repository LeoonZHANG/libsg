/**
 * charset.c
 * Char encoding detective and transcoding.
 */

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <iconv.h>
/*#include <charsetdetect.h>*/
#include <sg/sg.h>
#include <sg/str/charset.h>

/*
const char *sg_charset_detect(const char *str, const uint64_t str_len)
{
    csd_t csd;
    int rc;

    csd = csd_open();
    SG_ASSERT_MSG(csd != (csd_t)-1, "csd_open failed");

    rc = csd_consider(csd, str, str_len);
    if (rc < 0)
        return NULL;
    return csd_close(csd);
}*/

/**
 @param cd 是转换描述用的结构体
 @param inbuf 是指向输入数据的指针的指针, 函数工作时，*inbuf会不断自增扫描
 @param inbytesleft *inbytesleft 则表示输入数组还剩多少字节，注意单位是字节, 当 *inbytesleft 为 0 时，表示输入完毕
 @param outbuf 函数工作时，*outbuf会不断向后移动填写数据
 @param outbytesleft *outbytesleft为0后，会触发剩余空间不足的错误，方便你重新分配空间
 @remark 这个函数工作时，所有的指针都在变化
 size_t iconv (iconv_t cd,
              const char **inbuf, size_t *inbytesleft,
              char **outbuf, size_t *outbytesleft);
*/

/**
 * http://www.ibm.com/support/knowledgecenter/ssw_i5_54/apis/iconv.htm
 */
bool sg_charset_conv(const char *from_charset, const char *to_charset,
        const void *from_str, const uint64_t from_str_len,
        const void *out_buf, const uint64_t out_buf_size, uint64_t *real_out_str_len)
{
    size_t rc;
    iconv_t ict;
    size_t in_bytes_left = (size_t)from_str_len;
    size_t out_bytes_left = (size_t)out_buf_size;

    SG_ASSERT(from_charset && strlen(from_charset) > 0);
    SG_ASSERT(to_charset && strlen(to_charset) > 0);
    SG_ASSERT(from_str && from_str_len > 0);
    SG_ASSERT(out_buf && out_buf_size > 0);

    ict = iconv_open(to_charset, from_charset);
    if (!ict) {
        sg_log_err("iconv from %s to %s open failure.", from_charset, to_charset);
        return false;
    }

    errno = 0;
    rc = iconv(ict, (char **)&from_str, &in_bytes_left, (char **)&out_buf, &out_bytes_left);
    if (rc == -1) {
        if (errno == E2BIG)
            sg_log_err("iconv failure, out buffer is not big enough, please make it bigger and try again\n");
        else
            sg_log_err("iconv failure, %s.", strerror(errno));
        return false;
    }

    /* FIXME: is real_out_str_len right? whether unicode terminator is still one byte? */
    *real_out_str_len = out_buf_size - out_bytes_left;
    iconv_close(ict);
    return true;
}