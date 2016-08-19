/*
 * @file json_check.c
 * @author wangwei
 * @brief json validator
 */

#include <string.h>
#include "../../3rdparty/json_checker/JSON_checker.h"
#include <sg/str/json_check.h>
#include <sg/util/assert.h>

int sg_json_check(const char *json, int check_depth)
{
    JSON_checker checker;
    size_t i, len;

    sg_assert(json);
    sg_assert(check_depth > 0);

    len = strlen(json);
    if (len == 0) /* content null */
        return -1;

    checker = new_JSON_checker(check_depth);

    for (i = 0; i < len; i++) {
        if (json[i] <= 0)
            break;
        if (!JSON_checker_char(checker, json[i])) /* syntax error */
            return -1;
    }

    if (!JSON_checker_done(checker)) /* syntax error */
        return -1;

    return 0;
}