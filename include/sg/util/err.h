/*
 * @file err.h
 * @author wangwei
 * @brief error subsystem
 */

#ifndef LIBSG_ERR_H
#define LIBSG_ERR_H

#include "../sys/os.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum sg_err {
    SG_ERR_MIN = 0,
    SG_OK = 0,
    SG_ERR,
    SG_ERR_MALLOC_FAIL,
    SG_ERR_NULL_PTR,
    SG_ERR_MAX
} sg_err_t;

/* you can add custom errno and errmsg in your project */
void sg_err_add_custom(int err_no, const char *err_msg);

/* transform err number to error message */
const char *sg_err_fmt(sg_err_t err_no);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_ERR_H */