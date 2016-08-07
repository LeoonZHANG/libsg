/*
 * type.h
 * Author: wangwei.
 * Data pool for common types.
 */

#include <stddef.h> /* size_t */

typedef void (*sg_type_data_func_t)(void *data, size_t size, void *ctx);

typedef void (*sg_type_str_func_t)(const char *str, void *ctx);

typedef void (*sg_type_conn_on_open)(void *ctx);

typedef void (*sg_type_conn_on_msg)(void *data, size_t size, void *ctx);

typedef void (*sg_type_conn_on_close)(void *ctx);

typedef void (*sg_type_conn_on_err)(const char *err_msg, void *ctx);