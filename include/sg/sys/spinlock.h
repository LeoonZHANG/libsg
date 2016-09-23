/**
 * rwlock.h
 * 参考https://github.com/malbrain/rwlock
 * https://github.com/ScrimpyCat/RWLock
 * 跨平台高性能rwlock封装,一个h文件,多个c文件,如下。如果需要,可以调整c文件的分布结构。
      rwlock_atom.c    基于https://github.com/malbrain/rwlock/blob/master/readerwriter.c等的封装或者实现,可以#include这个c文件,这样尽量不修改其源码
      rwlock_api.c 系统读写锁系统API的封装,linux的pthread,windows7以及以上也有,
      rwlock_winxp.c   winxp需要其他方法模拟,网上大把,找个简单的即可,性能无所谓
      除了单元测试,还要做atom与系统api两种实现之间的对比
 */
#ifndef LIBSG_SPINLOCK_H
#define LIBSG_SPINLOCK_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_spinlock_real sg_spinlock_t;

sg_spinlock_t *sg_spinlock_alloc(void);

bool sg_spinlock_lock(sg_spinlock_t *self);

bool sg_spinlock_unlock(sg_spinlock_t *self);

void sg_spinlock_free(sg_spinlock_t *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_SPINLOCK_H */
