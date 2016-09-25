#ifndef __UDP_H__
#define __UDP_H__
#ifdef __cplusplus
extern "C"	{
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <uv.h>
#include "tcp_common.h"

//-----------------------------------------------
/* 本头文件定义UDP 通信 相关的数据结构以及接口函数
   与TCP不同, UDP无连接操作, 因此不需要区分client和server
*/  

typedef struct uvx_udp_s uvx_udp_t;

/* 
功能定义: UDP 节点收到数据回调处理函数 
是否必须定义: 可选定义, 用户未定义回调时, 数据将被丢弃
参数解释: 
uvx_udp_t* xudp  udp句柄, 具体请参考uvx_udp_t内容定义
void* data : 收到的数据流指针首地址, 该字节流长度由datalen指定
ssize_t datalen: 收到的数据流的字节长度
const struct sockaddr* addr   接收到的数据的来源端地址, 包括ip地址与端口
unsigned int flags  当前数据的指示信息, 当前版本只支持UV_UDP_PARTIAL(2)和0两种取值
                    如果flags取值为0, 则说明当前接收UDP报文正常
                    如果flags取值为UV_UDP_PARTIAL(2), 说明当前系统UDP接收缓冲区太小,
                    接收到的报文已经被截断, 用户收到的报文是部分报文, 需要调节UDP socket缓冲区大小
*/
typedef void (*UVX_UDP_ON_RECV) (uvx_udp_t* xudp, 
                                 void* data, 
                                 ssize_t datalen, 
                                 const struct sockaddr* addr, 
                                 unsigned int flags);

/* udp 实例配置信息 */
typedef struct uvx_udp_config_s {
    char name[32];              /* udp实例名, 名字默认为 udp-随机数字 */
    UVX_UDP_ON_RECV on_recv;    /* udp数据报文接收回调处理函数, 可选 */
    FILE* log_out;
    FILE* log_err;
} uvx_udp_config_t;

/* udp 实例数据结构 */
struct uvx_udp_s {
    uv_loop_t* uvloop;          /* udp句柄对应的uvloop句柄 */ 
    uv_udp_t   uvudp;           /* udp句柄对应的uv_udp_t */ 
    uvx_udp_config_t config;    /* udp句柄对应的uvx_udp_config_t */ 
    void* data;                 /* 用户扩展数据区, 可自定义数据, 在回调函数中处理 */
};


/*
函数功能描述: 生成UDP通信节点的默认配置信息, 用户可用该函数先生成一个默认配置的
              uvx_udp_config_t, 然后根据自己需要修改其中部分参数
参数描述:     uvx_udp_t* xudp  用户定义的uvx_udp_t指针
返回值描述:   返回一个填充默认值的uvx_udp_config_t结构
*/
uvx_udp_config_t uvx_udp_default_config(uvx_udp_t* xudp);


/*
函数功能描述: 启动UDP通信节点
参数描述:     uvx_udp_t* xudp  用户定义的uvx_udp_t指针
              uv_loop_t* loop  用户定义的uv_loop_t指针, 通常调用libuv接口函数获得
              const char* ip   UDP 通信节点侦听的ip地址, 如果想侦听系统上所有的ip地址,
                               侦听0.0.0.0
              int port         UDP 通信节点侦听的端口
              uvx_udp_config_t config  UDP通信节点的配置信息
返回值描述:   成功返回 1
              失败返回 0
*/
int uvx_udp_start(uvx_udp_t* xudp, uv_loop_t* loop, 
                  const char* ip, int port, 
                  uvx_udp_config_t config);


/*
函数功能描述: UDP通信节点对外发送数据
参数描述:     uvx_udp_t* xudp  用户定义的uvx_udp_t指针
              const char* ip   发送数据目的ip地址, 支持ipv4和ipv6
              int port         发送数据目的端口
              uvx_udp_config_t config  UDP通信节点的配置信息
              void* data   用户待发送的数据首地址, 该地址必须为用户通过
             malloc申请的堆内存, 不能是临时的变量内存; 用户在调用uvx_udp_send_to_ip
             后不能再对此块内存做任何处理, 内存释放由组件内部负责完成;
              unsigned int size      用户待发送的数据字节数量              
返回值描述:   成功返回 1
              失败范围 0
*/
int uvx_udp_send_to_ip(uvx_udp_t* xudp, 
                       const char* ip, int port, 
                       const void* data, unsigned int datalen);
/*
函数功能描述: UDP通信节点对外发送数据
参数描述:     uvx_udp_t* xudp  用户定义的uvx_udp_t指针
              const struct sockaddr* addr  发送数据目的通信地址
              uvx_udp_config_t config  UDP通信节点的配置信息
              void* data   用户待发送的数据首地址, 该地址必须为用户通过
             malloc申请的堆内存, 不能是临时的变量内存; 用户在调用uvx_udp_send_to_ip
             后不能再对此块内存做任何处理, 内存释放由组件内部负责完成;
              unsigned int size      用户待发送的数据字节数量              
返回值描述:   成功返回 1
              失败范围 0
*/                       
int uvx_udp_send_to_addr(uvx_udp_t* xudp, 
                         const struct sockaddr* addr, 
                         const void* data, unsigned int datalen);




/*
函数功能描述: 关闭UDP通信节点
参数描述:     uvx_udp_t* xudp  用户定义的uvx_udp_t指针            
返回值描述:   成功返回 1
              失败范围 0
*/
int uvx_udp_shutdown(uvx_udp_t* xudp);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
