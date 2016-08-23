/*
 * dial.h
 * Author: wangwei.
 * Windows dial-up connection management.
 */

#ifndef LIBSG_DIAL_H
#define LIBSG_DIAL_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Search for existent dial-up connection by name. */
bool sg_dial_find_conn(const char *name);

/* Start a dial-up connection by its name. */
bool dial_open_conn(const char *name);

/* Close a dial-up connection by its name. */
bool dial_close_conn(const char *name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_DIAL_H */
