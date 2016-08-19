/*
 * regex.c
 * Author: wangwei.
 * Regular expression library based on pcre.
 */

#include <stdio.h>
#include <string.h>
#include <pcre.h>
#include <sg/util/assert.h>
#include <sg/str/regex.h>
#include <sg/util/log.h>
#include <sg/util/def.h>

int regex_exec_real(char *src, sg_pattern *pat, sg_regex_callback cb, void *context, sg_vlstr_list *vl);

sg_pattern *sg_regex_make_pattern(const char *pattern_str)
{
    pcre *res;
    const char *err_msg = NULL;
    int err_offset = 0;

    assert(pattern_str);

    res = pcre_compile(pattern_str, 0, &err_msg, &err_offset, NULL);
    if (!res) {
        sg_log_err("PCRE compilation failed at offset %d: %s.",
                   err_offset, err_msg);
        return NULL;
    }

    return (sg_pattern *)res;
}

int sg_regex_exec(char *src, sg_pattern *pat, sg_regex_callback cb, void *context)
{
    assert(src);
    assert(pat);
    assert(cb);

    return regex_exec_real(src, pat, cb, context, NULL);
}

sg_vlstr_list *sg_regex_exec2(char *src, sg_pattern *pat)
{
    int ret;
    sg_vlstr_list *vl;

    assert(src);
    assert(pat);

    vl = sg_vlstr_list_alloc();
    if (!vl)
        return NULL;

    ret = regex_exec_real(src, pat, NULL, NULL, vl);

    if (ret < 0) {
        sg_vlstr_list_free(&vl);
        return NULL;
    }

    return vl;
}

void sg_regex_free_pattern(sg_pattern **pat)
{
    pcre **p = (pcre **)pat;

    assert(p);
    if (!(*p))
        return;

    pcre_free(*p);
    *p = NULL;
}

sg_vlstr_list *sg_regex_match(char *src, const char *pattern_str)
{
    sg_pattern *pat;
    sg_vlstr_list *vl;

    assert(src);
    assert(pattern_str);
    if (strlen(src) == 0 || strlen(pattern_str) == 0)
        return NULL;

    pat = sg_regex_make_pattern(pattern_str);
    if (!pat)
        return NULL;

    vl = sg_regex_exec2(src, pat);
    sg_regex_free_pattern(&pat);

    return vl;
}

/* Matches a compiled regular expression against a given subject string. */
int regex_exec_real(char *src, sg_pattern *pat, sg_regex_callback cb, void *context, sg_vlstr_list *vl)
{
    int i;
    int rc;
    int res_max_size = 10; /* max result size of output vector */
    int *ov = NULL;        /* output vector */
    int ov_cnt = 0;        /* should be a multiple of 3 */
    int substr_len;
    char *substr;

    assert(src);
    assert(pat);
    assert(cb || vl);

    while (1) {
        SAFE_FREE(ov);
        ov_cnt += 3 * res_max_size;
        ov = (int *)malloc(ov_cnt * sizeof(int));
        if (!ov) {
            sg_log_err("Vec malloc failure.");
            return -1;
        }
        rc = pcre_exec((const pcre *)pat, NULL, src, (int)strlen(src), 0, 0, ov, ov_cnt);

        /* Output vector wasn't big enough for captured substring(s). */
        if (rc == 0)
            continue;

        /* No match (PCRE_ERROR_NOMATCH: -1) or error (rc < -1). */
        if (rc < 0)
            break;

        /* Output results stored in the output vector. */
        for (i = 1; i < rc; i++) {
            substr = src + ov[2 * i];
            substr_len = ov[2 * i + 1] - ov[2 * i];
            if (cb)
                cb(substr, substr_len, context);
            if (vl)
                sg_vlstr_list_push2(vl, substr, substr_len);
            /* printf("%2d: %.*s\n", i, substr_len, substr); */
        }
        break;
    }

    SAFE_FREE(ov);

    return (rc == PCRE_ERROR_NOMATCH) ? 0 : rc;
}