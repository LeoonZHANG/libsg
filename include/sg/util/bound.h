/*
 * bound.h
 * Author: wangwei.
 * Bounds checking constant.
 */

#ifndef LIBSG_BOUND_H
#define LIBSG_BOUND_H

#include "../sys/os.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define BD_INT8_MIN    0x80

#define BD_INT16_MIN   0x8000

#define BD_INT32_MIN   0x80000000

#define BD_INT64_MIN   0x8000000000000000

#define BD_UINT8_MIN   0

#define BD_UINT16_MIN  0

#define BD_UINT32_MIN  0

#define BD_UINT64_MIN  0

#define BD_SIZE_T_MIN  0

#define BD_INT8_MAX    0x7f

#define BD_INT16_MAX   0x7fff

#define BD_INT32_MAX   0x7fffffff

#define BD_INT64_MAX   0x7fffffffffffffff

#define BD_UINT8_MAX   0xff

#define BD_UINT16_MAX  0xffff

#define BD_UINT32_MAX  0xffffffff

#define BD_UINT64_MAX  0xffffffffffffffff

#ifdef OS_64
# define BD_SIZE_T_MAX  0xffffffffffffffff
#else
# define BD_SIZE_T_MAX  0xffffffff
#endif



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_BOUND_H */
