/*
 * sg.h
 * Author: wangwei.
 * Libsg public module.
 */

#ifndef LIBSG_SG_H
#define LIBSG_SG_H

#include "util/cl.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

inline const char *sg_version(void)
{
    return SG_CL_DATETIME_STRING;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_SG_H */
