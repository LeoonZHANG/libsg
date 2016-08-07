/*
 * atom.h
 * Author: wangwei.
 * Atomic operation.
 */

#ifndef LIBSG_ATOM_H
#define LIBSG_ATOM_H

#include "../sys/os.h"
#ifdef OS_WIN
# include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef OS_WIN
    #define sg_atom_cmp_swap_long(ptr, old_val, new_val)   (InterlockedCompareExchange((long volatile *)ptr, new_val, old_val) == old_val)
    #define sg_atom_cmp_swap_ptr(ptr, old_val, new_val)    (InterlockedCompareExchangePointer((PVOID volatile *)ptr, new_val, old_val) == old_val)
    #define sg_atom_inc(ptr)                               InterlockedIncrement((long volatile *)ptr)
    #define sg_atom_inc_val(ptr, val)                      _InterlockedAdd16((long volatile *)ptr, val)
    #define sg_atom_dec(ptr)                               InterlockedDecrement((long volatile *)ptr)
    #define sg_atom_dec_val(ptr, val)                      _InterlockedAdd16((long volatile *)ptr, 0 - val)
    #define sg_atom_sync()                                 MemoryBarrier()
#else
    #define sg_atom_cmp_swap_long(ptr, old_val, new_val)   __sync_bool_compare_and_swap(ptr, old_val, new_val)
    #define sg_atom_cmp_swap_ptr(ptr, old_val, new_val)    __sync_bool_compare_and_swap(ptr, old_val, new_val)
    #define sg_atom_inc(ptr)                               __sync_add_and_fetch(ptr, 1)
    #define sg_atom_inc_val(ptr, val)                      __sync_add_and_fetch(ptr, val)
    #define sg_atom_dec(ptr)                               __sync_sub_and_fetch(ptr, 1)
    #define sg_atom_dec_val(ptr, val)                      __sync_sub_and_fetch(ptr, val)
    #define sg_atom_and(ptr, val)                          __sync_and_and_fetch(ptr, val)
    #define sg_atom_or(ptr, val)                           __sync_or_and_fetch(ptr, val)
    #define sg_atom_sync()                                 __sync_synchronize()
#endif

/* #define sg_atom_f_inc(ptr)                           __sync_fetch_and_add(ptr, 1) */
/* #define sg_atom_f_inc_val(ptr, val)                  __sync_fetch_and_add(ptr, val) */
/* #define sg_atom_f_dec(ptr)                           __sync_fetch_and_sub(ptr, 1) */
/* #define sg_atom_f_dec_val(ptr, val)                  __sync_fetch_and_sub(ptr, val) */
/* #define sg_atom_f_and(ptr, val)                      __sync_fetch_and_and(ptr, val) */
/* #define sg_atom_f_or(ptr, val)                       __sync_fetch_and_or(ptr, val) */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_ATOM_H */
