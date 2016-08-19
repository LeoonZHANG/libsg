/*
 * module.h
 * Author: wangwei.
 * APIs about modules.
 */

#ifndef LIBSG_MODULE_H
#define LIBSG_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Get current working directory which terminates in '/'. */
int sg_cur_dir(char *buf, size_t buf_len);

/* Get module filename path. */
int sg_module_path(char *buf, size_t buf_len);

/* Get module directory end with '/'. */
int sg_module_dir(char *buf, size_t buf_len);

/* Get module short name without suffix. */
int sg_module_name(char *buf, size_t buf_len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_MODULE_H */