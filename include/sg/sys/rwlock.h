/**
 * rwlock.h
 * 参考
 * https://github.com/malbrain/rwlock
 * https://github.com/ScrimpyCat/RWLock
 * https://github.com/apache/apr/blob/trunk/include/apr_thread_rwlock.h
 * 跨平台高性能rwlock封装,一个h文件,多个c文件,如下。如果需要,可以调整c文件的分布结构。
      atom实现的rwlock    基于https://github.com/malbrain/rwlock/blob/master/readerwriter.c等的封装或者实现,可以#include这个c文件,这样尽量不修改其源码
      api的rwlock 系统读写锁系统API的封装,linux的pthread,windows7以及以上也有,
      apr的rwlock
      winxp的rwlock   winxp需要其他方法模拟,网上大把,找个简单的即可,性能无所谓
      除了单元测试,还要做atom与系统api两种实现之间的对比
      同一个接口的多种实现方式一律放在同一个函数的内部,用宏定义区分,方便观看的时候对比其实现差异
      性能测试写好以后, 三个平台下都要跑一下
 */
#ifndef LIBSG_RWLOCK_H
#define LIBSG_RWLOCK_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_rwlock_real sg_rwlock_t;

sg_rwlock_t *sg_rwlock_alloc(void);
void sg_rwlock_free(sg_rwlock_t *self);

bool sg_rwlock_lock_r(sg_rwlock_t *self);
bool sg_rwlock_unlock_r(sg_rwlock_t *self);
bool sg_rwlock_lock_w(sg_rwlock_t *self);
bool sg_rwlock_unlock_w(sg_rwlock_t *self);
bool sg_rwlock_try_lock_r(sg_rwlock_t *self);
bool sg_rwlock_try_lock_w(sg_rwlock_t *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_RWLOCK_H */
