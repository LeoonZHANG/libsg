/*
 * assert.h
 * Author: wangwei.
 * Assert.
 */

#ifndef LIBSG_ASSERT_H
#define LIBSG_ASSERT_H

#include "log.h"

/* keep assert open */
#ifdef NDEBUG
# undef NDEBUG
# include <assert.h>
# define NDEBUG
#else
# include <assert.h>
#endif /* #ifdef NDEBUG */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define sg_assert assert

//#define SG_ASSERT_ERRNO
//#define SG_ASSERT_ALLOC
//#define SG_ASSERT_PTR

#define SG_ASSERT_MSG(exp, fmt, args...) \
do { \
    if (!(exp)) { \
        sg_log(SGLOGLEVEL_CRIT, \
               "Assert \'"#exp"\' false. "fmt, \
               ##args); \
    } \
    sg_assert(exp); \
} while(0)


/* if exp equals false, logging by 'fmt' and 'args...',
   and, abort process when assert opened up,
   otherwise execute 'response' operation. */
/*#define ensure_sample(exp, response, fmt, args...) \
do { \
    if (!(exp)) { \
        sg_log(LOGLEVEL_CRIT, \
               "Conditional filter \'"#exp"\' false. "fmt, \
               ##args); \
    } \
    assert(exp); \
    if (!(exp)) { \
        response; \
    } \
} while(0)*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_ASSERT_H */
