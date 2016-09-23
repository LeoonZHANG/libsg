/*
 * trick.h
 * C tricks.
 */

#ifndef LIBSG_TRICK_H
#define LIBSG_TRICK_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* 扩栈,系统栈在windows下有限,爆了怎么办？*/
#pragma comment(linker, "/STACK:102400000,102400000")

inline int sg_trick_abs(int x) /* 消除分支预测 */
{
    int y = x >> 31;
    return (x + y) ^ y;
}

inline int sg_trick_max(int x, int y) /* 消除分支预测 */
{
    return y&((x-y)>>31)|x&~((x-y)>>31);
}

inline int sg_trick_average(int x, int y) /* 避免相加溢出 */
{
    return (x & y) + ((x ^ y) >> 1);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_TRICK_H */
