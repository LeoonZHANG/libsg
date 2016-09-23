/**
 * type.h
 * type definition data pool.
 */

#ifndef LIBSG_TYPE_H
#define LIBSG_TYPE_H

#include <sg/sg.h>

#ifndef __cplusplus

#ifdef SG_OS_WINDOWS /* fow WINDOWS */
# ifndef bool
typedef unsigned char bool
# endif
# ifndef false
#  define false 0
# endif
# ifndef true
#  define true  1
# endif
#else /* for POSIX */
# include <stdbool.h> /* bool */
# include <sys/types.h> /* uint8_t...... */
#endif /* SG_OS_WINDOWS */

#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_TYPE_H */
