
/*
 * tcp_common.h
 * High concurrency tcp server library based on libuv.
 */

#ifndef __TCP_COMMON_H__
#define __TCP_COMMON_H__
#ifdef __cplusplus
extern "C"	{
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <uv.h>

//-----------------------------------------------
// 本文件声明公共接口

/*
函数功能描述: 把一个struct sockaddr结构的地址转换为可读的字符串ip地址和端口号
参数描述:     const struct sockaddr* addr  输入的地址
              char* ipbuf  输入地址转换后的ip地址字符串,C风格, 以0结尾
                           ipbuf的内存需要调用者进行分配
              int buflen   ipbuf的内存长度, 需要调用者进行合理设置
                           ipv4地址建议不小于16, ipv6地址建议不小于40
              int* port    输入地址转换后的端口号, 得到的结果为主机序
返回值描述:   操作成功时返回的值等于ipbuf
              操作失败时返回NULL
*/
const char* cmm_get_ip_port(const struct sockaddr* addr, 
                            char* ipbuf, 
                            int buflen, 
                            int* port);


/*
函数功能描述: 把一个struct sockaddr结构的地址转换为二进制的ip地址和端口号
参数描述:     const struct sockaddr* addr  输入的地址
              char* ipbuf  输入地址转换后的ip地址字符串, ipbuf的内存需要调用者进行分配                           
                           ipv4地址缓存大小为4, ipv6地址缓存大小为16
              int* port    输入地址转换后的端口号, 得到的结果为主机序
返回值描述:   操作成功时 > 0
              操作失败时返回0
*/
int cmm_get_raw_ip_port(const struct sockaddr* addr, unsigned char* ipbuf, int* port);


/*
函数功能描述: 把一个uv_tcp_t结构的句柄转换为可读的字符串的ip地址和端口号
参数描述:     uv_tcp_t* uvclient  输入的uv_tcp_t句柄指针
              char* ipbuf  输入地址转换后的ip地址字符串,C风格, 以0结尾
                           ipbuf的内存需要调用者进行分配
              int buflen   ipbuf的内存长度, 需要调用者进行合理设置
                           ipv4地址建议不小于16, ipv6地址建议不小于40
              int* port    输入地址转换后的端口号, 得到的结果为主机序
返回值描述:   操作成功时返回的值等于ipbuf
              操作失败时返回NULL

*/
const char* cmm_get_tcp_ip_port(uv_tcp_t* uvclient, char* ipbuf, int buflen, int* port);

/*
函数功能描述: 向一个已经打开的uv_stream_t中写入数据
参数描述:     uv_stream_t* stream  已经正常打开的uv_stream_t
              void* data   用户待发送数据的内存首地址, data的内存需要调用者进行分配
                           data的内存释放将在被调用函数内部完成, 因此调用者在调用
                           cmm_send_to_stream函数后, 不能再操作void* data指向的内存                           
              unsigned int size   待发送数据的字节长度

返回值描述:   操作成功时返回 1
              操作失败时返回 0
*/
int cmm_send_to_stream(uv_stream_t* stream, void* data, unsigned int size);



#if defined(_WIN32) && !defined(__GNUC__)
#include <stdarg.h>
/* Windows上的snprintf模拟函数 */ 
int snprintf(char* buf, size_t len, const char* fmt, ...);
#endif


#ifdef __cplusplus
} // extern "C"
#endif

#endif

