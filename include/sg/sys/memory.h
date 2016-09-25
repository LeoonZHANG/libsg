/**
 * memory.h
 * fast malloc/free/memcpy based on tcmalloc/jemalloc/fastmemcpy.
 * 很乱, 需要梳理, 另外, 最好可以实现-lsg就自动-ltcmalloc或者-ljemalloc
 */

#ifndef LIBSG_MEMORY_H
#define LIBSG_MEMORY_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "../../../3rdparty/FastMemcpy/FastMemcpy.h"

/* 通过宏定义决定是使用tcmalloc还是jemalloc */
#define USE_TCMALLOC /* #define USE_JEMALLOC */

#ifdef  USE_JEMALLOC
#include "../../../3rdparty/jemalloc/jemalloc.h"
#endif //  USE_JEMALLOC


#ifdef _MSC_VER
#define FM_JE_MALLOC		je_malloc
#define FM_JE_FREE			je_free
#else
#define FM_JE_MALLOC		malloc
#define FM_JE_FREE			free
#endif

/* tc采用的动态替换的原则，所以不需要用外部符号，直接用标准库的malloc和free即可
   在平台编码上，没有任何区别 */
#ifdef _MSC_VER
#define FM_TC_MALLOC		malloc
#define FM_TC_FREE			free
#else
#define FM_TC_MALLOC		malloc
#define FM_TC_FREE			free
#endif



#ifdef USE_TCMALLOC

#define SG_MALLOC_FUN			FM_TC_MALLOC
#define SG_FREE_FUN				FM_TC_FREE

#elif defined(USE_JEMALLOC)

#define SG_MALLOC_FUN			FM_JE_MALLOC
#define SG_FREE_FUN				FM_JE_FREE

#else
/* 使用标准库的接口 */
#define SG_MALLOC_FUN			malloc
#define SG_FREE_FUN				free
#endif

#define SG_MEMCPY_FUN			memcpy_fast


#define sg_malloc(SIZE)				SG_MALLOC_FUN(SIZE)
#define sg_free(POINTER)			SG_FREE_FUN(POINTER)
	/* memcpy：use skywind's memcopy */
#define sg_memcpy(DST, SRC, SIZE)	SG_MEMCPY_FUN(DST,SRC,SIZE)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_MEMORY_H */