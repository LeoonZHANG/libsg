/**
 * sg.h
 * Libsg public module.
 */

#ifndef LIBSG_SG_H
#define LIBSG_SG_H

/* frequently used header files */
#include "sys/os.h"
#include "sys/type.h"
#include "sys/limit.h"
#include "util/assert.h"
#include "util/log.h"
#include "util/err.h"
#include "util/compiler.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

inline const char *sg_version(void)
{
    return SG_COMPILER_DATETIME_STRING;
}

/* By default, we use the standard "extern" declarations. */
#ifndef LIBSG_EXP_DECL
# ifdef __cplusplus
#  define LIBSG_EXP_DECL  extern "C"
# else
#  define LIBSG_EXP_DECL  extern
# endif
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_SG_H */
