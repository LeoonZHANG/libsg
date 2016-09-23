/**
 * limit.h
 * Limitation data pool.
 */

#ifndef LIBSG_LIMIT_H
#define LIBSG_LIMIT_H

#include <sg/sg.h>

#define SG_LIMIT_PATH_MAX 1024

#define SG_LIMIT_URL_MAX  2038 /* default max url length of IE */

#define SG_LIMIT_IPV4_MAX 16

#define SG_LIMIT_IPV6_MAX 46

#define SG_LIMIT_IP_MAX SG_LIMIT_IPV6_MAX















#define SG_LIMIT_INT8_MIN    0x80

#define SG_LIMIT_INT16_MIN   0x8000

#define SG_LIMIT_INT32_MIN   0x80000000

#define SG_LIMIT_INT64_MIN   0x8000000000000000

#define SG_LIMIT_UINT8_MIN   0

#define SG_LIMIT_UINT16_MIN  0

#define SG_LIMIT_UINT32_MIN  0

#define SG_LIMIT_UINT64_MIN  0

#define SG_LIMIT_SIZE_T_MIN  0

#define SG_LIMIT_INT8_MAX    0x7f

#define SG_LIMIT_INT16_MAX   0x7fff

#define SG_LIMIT_INT32_MAX   0x7fffffff

#define SG_LIMIT_INT64_MAX   0x7fffffffffffffff

#define SG_LIMIT_UINT8_MAX   0xff

#define SG_LIMIT_UINT16_MAX  0xffff

#define SG_LIMIT_UINT32_MAX  0xffffffff

#define SG_LIMIT_UINT64_MAX  0xffffffffffffffff

#ifdef SG_OS_BITS_64
# define SG_LIMIT_SIZE_T_MAX  0xffffffffffffffff
#else
# define SG_LIMIT_SIZE_T_MAX  0xffffffff
#endif

#endif /* LIBSG_LIMIT_H */