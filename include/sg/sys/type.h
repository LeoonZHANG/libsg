/**
 * type.h
 * type definition data pool.
 */

#ifndef LIBSG_TYPE_H
#define LIBSG_TYPE_H

#include "os.h"

#include <stddef.h> /* size_t */
#include <stdlib.h> /* size_t */
#include <stdint.h> /* uint8_t uint32_t uint64_t */

#ifndef SG_OS_WINDOWS
# include <sys/types.h> /* uchar ushort uint ulong ...... */
#endif

/* bool */
#ifndef __cplusplus
# ifdef SG_OS_WINDOWS /* fow WINDOWS */
#  ifndef bool
    typedef unsigned char bool
#  endif
#  ifndef false
#   define false 0
#  endif
#  ifndef true
#   define true  1
#  endif
# else /* for POSIX */
#  include <stdbool.h> /* bool */
# endif /* SG_OS_WINDOWS */
#endif /* __cplusplus */

#endif /* LIBSG_TYPE_H */
