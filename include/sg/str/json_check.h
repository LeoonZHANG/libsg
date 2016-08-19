/*
 * @file json_check.h
 * @author wangwei
 * @brief JSON validator
 */

#ifndef LIBSG_JSON_CHECK_H
#define LIBSG_JSON_CHECK_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Check json is valid or not.
 * If valid, it returns 0.
 */
int sg_json_check(const char *json, int check_depth);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_JSON_CHECK_H */