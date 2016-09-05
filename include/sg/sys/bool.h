/*
 * bool.h
 * Author: wangwei.
 * bool definition.
 */

#ifndef LIBSG_BOOL_H
#define LIBSG_BOOL_H

#ifndef __cplusplus

#include "../sys/os.h"

#ifdef OS_WIN
typedef int bool
#define false 0
#define true  1
#else
#include <stdbool.h>
#endif /* OS_WIN */

#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_BOOL_H */
