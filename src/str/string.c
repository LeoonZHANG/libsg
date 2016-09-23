/**
 * string.c
 * String process.
 */

#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sg/sg.h>
#include <sg/str/string.h>

int sg_str_to_lower(char *str)
{
    int i;
    size_t len;

    SG_ASSERT(str);

    for (i = 0, len = strlen(str); i < len; i++)
        str[i] = (char)tolower(str[i]);

    return 0;
}

int sg_str_to_upper(char *str)
{
    int i;
    size_t len;

    SG_ASSERT(str);

    for (i = 0, len = strlen(str); i < len; i++)
        str[i] = (char)toupper(str[i]);

    return 0;
}

bool sg_str_is_hex(const char *src)
{
    int i;
    size_t len;

    SG_ASSERT(src);

    len = strlen(src);
    if (len == 0)
        return false;

    for (i = 0; i < len; i++)
        if (isxdigit(src[i]) == 0) /* not a hex char */
            return false;

    return true;
}

bool sg_str_is_decimal(const char *src)
{
    int i;
    size_t len;

    SG_ASSERT(src);

    len = strlen(src);
    if (len == 0)
        return false;

    for (i = 0; i < len; i++)
        if (isdigit(src[i]) == 0 && src[i] != '.' && src[i] != '-') /* not a numeric char / '.' / '-' */
            return false;

    if (strstr(src, ".."))
        return false;

    /* How many '.' in str. */

    return true;
}

int sg_str_to_bool(const char *src, bool *out)
{
    char *swap;
    int  res = 0;

    SG_ASSERT(src);
    SG_ASSERT(out);

    swap = strdup(src);
    if (!swap)
        return -1;

    sg_str_to_upper(swap);
    if (strcmp(swap, "TRUE") == 0)
        *out = true;
    else if (strcmp(swap, "FALSE") == 0)
        *out = false;
    else
        res = -1;

    free(swap);

    return res;
}

int sg_decimal_str_to_double(const char *src, double *out)
{
    SG_ASSERT(src);
    SG_ASSERT(out);

    if (!sg_str_is_decimal(src))
        return -1;

    *out = atof(src);

    return 0;
}

int sg_decimal_str_to_int(const char *src, int *out)
{
    SG_ASSERT(src);
    SG_ASSERT(out);

    if (!sg_str_is_decimal(src))
        return -1;

    *out = atoi(src);

    return 0;
}

int sg_str_del_chr(char *str, const char chr)
{
    int res = 0;
    char *swap = str;

    SG_ASSERT(str);

    while (*swap) {
        if (*swap != chr)
            *str++ = *swap;
        else
            res++;
        swap++;
    }
    *str = '\0';

    return res;
}

int sg_str_rep_chr(char *str, const char old_chr, const char new_chr)
{
    int res = 0;

    SG_ASSERT(str);
    SG_ASSERT(strlen(str) > 0);
    SG_ASSERT(old_chr != '\0');
    SG_ASSERT(new_chr != '\0');

    while (*str) {
        if (*str == old_chr) {
            *str = new_chr;
            res++;
        }
        str++;
    }

    return res;
}

int sg_str_del_str(char *str, const char *del)
{
    char *p, *q;
    char *src, *dst;

    SG_ASSERT(str);
    SG_ASSERT(del);
    if (strlen(str) == 0 || strlen(del) == 0)
        return -1;
    if (strlen(del) > strlen(str))
        return -1;

    dst = str;
    src = str;

    while (*src != '\0') {
        p = src;
        q = (char *)del;
        while (*p == *q && *q != '\0') {
            p++;
            q++;
        }
        if (*q == '\0')
            src = p;
        else
            *dst++ = *src++;
    }
    *dst = '\0';

    return 0;
}

char *sg_str_r_str(char *src, char *find)
{
    char *p;
    size_t src_len = strlen(src);
    size_t find_len = strlen(find);

    SG_ASSERT(src);
    SG_ASSERT(find);

    if (src_len == 0 || find_len == 0 || find_len > src_len)
        return NULL;

    for (p = src + src_len - 1; p >= src; p--)
        if ((*p == *find) && (memcmp(p, find, find_len) == 0))
            return p;

    return NULL;
}

size_t sg_str_sub_count(const char *src, const char *sub)
{
    char *src_temp = (char *)src;
    size_t sub_len = strlen(sub);
    size_t cnt = 0;

    if (sub_len > strlen(src))
        return 0;

    while (src_temp = strstr(src_temp, sub)) {
        cnt ++;
        src_temp += sub_len;
    }
    return cnt;
}