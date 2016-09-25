/**
 * charset.c
 * Char encoding detective and transcoding.
 */

#include <iconv.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sg/sg.h>
#include <sg/str/charset.h>

int sg_charset_conv(const char *from_charset, const char *to_charset,
                    const char *from_str, sg_vsstr_t *out_str)
{
    size_t rc;
    iconv_t ict;
    size_t from_len;
    char *tmp_out_str;
    size_t tmp_out_str_len;

    SG_ASSERT(from_charset);
    SG_ASSERT(strlen(from_charset) > 0);
    SG_ASSERT(to_charset);
    SG_ASSERT(strlen(to_charset) > 0);
    SG_ASSERT(from_str);
    SG_ASSERT((from_len = strlen(from_str)) > 0);
    SG_ASSERT(out_str);

    ict = iconv_open(to_charset, from_charset);
    if (!ict) {
        sg_log_err("iconv from %s to %s open failure.", from_charset, to_charset);
        return -1;
    }

    errno = 0;
    rc = iconv(ict, (char **)&from_str, &from_len, &tmp_out_str, &tmp_out_str_len);
    if (rc == -1) {
        sg_log_err("iconv failure, %s.", strerror(errno));
        //perror("iconv");
        return -1;
    }
    sg_vsstr_cpy(out_str, tmp_out_str);

    iconv_close(ict);
    return 0;
}