/*
 * assert.h
 * Author: wangwei.
 * Assert.
 */

#ifndef LIBSG_ASSERT_H
#define LIBSG_ASSERT_H

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

//#define sg_assert_errno
//#define sg_assert_int
//#define sg_assert_uint32


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
