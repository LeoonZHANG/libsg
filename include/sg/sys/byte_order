/**
 * byte_order.h
 * Byte order test.
 *
 *
 * 什么是字节序?
 * 顾名思义字节的顺序，大于一个字节类型的数据在内存中的存放顺序(一个字节的数据当然就无需谈顺序问题)。
 *
 * 字节序由什么决定?
 * 字节序和处理器架构有关,是硬件层的概念,和软件、编程语言无关。
 *
 * 什么时候需要考虑字节序?
 * 唯有在跨平台以及网络程序中字节序才是一个应该被考虑的问题。
 *
 * 有哪些字节序?
 * a) 高尾端(大端)Big-Endian: 用内存高位存放(数学上)数据的尾巴字节。
 * b) 低尾端(小端)Little-Endian: 用内存低位存放(数学上)数据的尾巴字节。
 * 高/低是指内存地址的高低, 尾巴是指数学上数据的尾巴(比如0x11223344中的0x44)
 *
 * 网络字节序是什么?
 * TCP/IP各层协议将字节序定义为Big-Endian，因此TCP/IP协议中使用的字节序通常称之为网络字节序。
 *
 * 可以举例说明吗?
 * 看看两种方式如何存储数据，假设从地址0x00000001处开始存储十六进制数0x12345678，那么
 * Big-endian 如此存放(按原来顺序存储)
 * 0x00000001           -- 12
 * 0x00000002           -- 34
 * 0x00000003           -- 56
 * 0x00000004           -- 78
 * Little-endian 如此存放(颠倒顺序储存)
 * 0x00000001           -- 78
 * 0x00000002           -- 56
 * 0x00000003           -- 34
 * 0x00000004           -- 12
 */



#ifndef LIBSG_BYTE_ORDER_H
#define LIBSG_BYTE_ORDER_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
 
inline bool sg_byte_order_is_big_endian(void)
{
    short data = 0x0011; /* high byte in data: 0x00, low byte|endian|tail in math: 0x11 */
    char endian_in_data = 0x11; /* endian|tail in data: 0x11 */
    char high_byte_in_mem = ((char *)&data)[1];  /* get high byte in memory */

    if (high_byte_in_mem == endian_in_data) /* big-endian */
        return true;
    else  /* little-endian */
        return false;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_BYTE_ORDER_H */
