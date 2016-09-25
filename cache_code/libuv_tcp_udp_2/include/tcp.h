/*
 * tcp.h
 * High concurrency tcp client library based on libuv.
 */


#ifndef __TCP_H__
#define __TCP_H__
#ifdef __cplusplus
extern "C"	{
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <uv.h>
#include "tcp_common.h"

/*
本头文件定义tcp client 相关的数据结构以及接口函数
*/

typedef struct tcp_client_s tcp_client_t;

/*tcp client 的回调处理函数接口定义 */

/* 
功能定义: TCP client connect server成功时回调处理函数 
是否必须定义: 可选定义, 用户未定义回调时连接将按照成功继续处理
参数解释: 
tcp_client_t* xclient  tcp client句柄, 具体请参考tcp_client_t内容定义
*/
typedef void (*TCP_C_ON_CONN_OK)        (tcp_client_t* xclient);    

/* 
功能定义: client connect server失败时回调处理函数 
是否必须定义: 可选定义, 用户未定义回调时连接将按照连接失败继续处理
参数解释: 
tcp_client_t* xclient  tcp client句柄, 具体请参考tcp_client_t内容定义
*/
typedef void (*TCP_C_ON_CONN_FAIL)      (tcp_client_t* xclient);

/* 
功能定义: client 连接关闭前的回调处理函数 
是否必须定义: 可选定义, 用户未定义回调时连接将按照连接继续关闭处理
参数解释: 
tcp_client_t* xclient  tcp client句柄, 具体请参考tcp_client_t内容定义
*/
typedef void (*TCP_C_ON_CONN_CLOSING)   (tcp_client_t* xclient);

/* 
功能定义: client 连接关闭完成后的回调处理函数 
是否必须定义: 可选定义
参数解释: 
tcp_client_t* xclient  tcp client句柄, 具体请参考tcp_client_t内容定义
*/
typedef void (*TCP_C_ON_CONN_CLOSE)     (tcp_client_t* xclient);

/* 
功能定义: client 收到数据时回调处理函数 
是否必须定义: 可选定义, 如果用户未定义该回调, 所有收到的数据将会被丢弃;
              如果用户定义了该回调函数, 可以通过函数的输入参数获取收到的数据
参数解释: 
tcp_client_t* xclient  tcp client句柄, 具体请参考tcp_client_t内容定义
void* data : 收到的数据流指针首地址, 该字节流长度由datalen指定
ssize_t datalen: 收到的数据流的字节长度
*/
typedef void (*TCP_C_ON_RECV)           (tcp_client_t* xclient, void* data, ssize_t datalen);

/* 
功能定义: client 心跳周期时间到达时的回调处理函数
该定时器从尝试创建连接开始启动, 心跳周期时长由tcp_client_config_t.heartbeat_interval_seconds
决定; 心跳周期函数为上层应用提供了一个简单的定时器, 用户可以在该函数中执行需要周期
进行的逻辑处理; 
同时, tcp客户端依靠该定时器在连接失败或连接异常断开时自动重新连接
是否必须定义: 可选定义, 如果用户未定义该回调, 则;
              如果用户定义了该回调函数, 可以通过函数的输入参数获取收到的数据
参数解释: 
tcp_client_t* xclient  tcp client句柄, 具体请参考tcp_client_t内容定义
unsigned int index  心跳周期回调函数执行次数, 从client调用connect函数开始计数, 
                    初始值为0, 调用一次心跳周期回调函数该值增加1;                    
*/
typedef void (*TCP_C_ON_HEARTBEAT)      (tcp_client_t* xclient, unsigned int index);

/*
tcp client 应用控制结构, 本结构为应用层最为重要的数据接口
*/
typedef struct tcp_client_config_s {
    char name[32];       /* 当前client的名称, 用户可自定义, 默认值为
                            tclient-随机数字, 主要用于日志显示不同的client */
    int retry_times;      /* tcp client连接失败后重试次数, 默认为5次*/
    float heartbeat_interval_seconds;   /* 心跳周期时长, 可通过小数点支持毫秒 */
    // callbacks
    TCP_C_ON_CONN_OK       on_conn_ok;  /* 连接创建成功回调, 可选 */
    TCP_C_ON_CONN_FAIL     on_conn_fail;/* 连接创建失败回调, 可选 */
    TCP_C_ON_CONN_CLOSING  on_conn_closing; /* 连接关闭前回调, 可选 */
    TCP_C_ON_CONN_CLOSE    on_conn_close;   /* 连接关闭完成后回调, 可选 */
    TCP_C_ON_RECV          on_recv;         /* 收到SERVER数据回调, 可选 */
    TCP_C_ON_HEARTBEAT     on_heartbeat;    /* 周期心跳回调函数, 可选 */
    // logs
    FILE* log_out;                          
    FILE* log_err;
} tcp_client_config_t;

/* TCP client 句柄数据结构, 每个TCP client对应一个tcp_client_s */
struct tcp_client_s {
    uv_loop_t* uvloop;            /* 该client句柄对应的uvloop句柄 */       
    uv_tcp_t   uvclient;          /* 该client句柄对应的uv_tcp_t client句柄 */ 
    uv_tcp_t*  uvserver;          /* 该client句柄对应的uv_tcp_t server句柄 */
    tcp_client_config_t config;   /* 该client句柄对应的client配置信息结构 */
    unsigned char privates[296];  /* 该client句柄对应的client内部管理信息 */
    void* data;                   /* 该client句柄对应的用户自定义数据区, 用户可
                                     自行申请内存处理 */
};
typedef struct tcp_client_s tcp_client_t;

/*
函数功能描述: 生成TCP client的默认配置信息, 用户可用该函数先生成一个默认配置的
              tcp_client_config_t, 然后根据自己需要修改其中部分参数
参数描述:     tcp_client_t* xclient  用户定义的tcp_client_t指针
返回值描述:   返回一个填充默认值的tcp_client_config_t结构
*/
tcp_client_config_t tcp_client_default_config(tcp_client_t* xclient);

/*
函数功能描述: TCP Client尝试连接TCP server
参数描述:     tcp_client_t* xclient  用户定义的tcp_client_t指针
              uv_loop_t* loop  用户定义的uv_loop_t指针, 通常调用libuv接口函数获得
              const char* ip   server的ip地址字符串, 支持ipv4和ipv6两种格式
                               ipv4样例 a.b.c.d
                               ipv6样例 2000:0000:0000:0000:0001:2345:6789:abcd
                                        或2000::1:2345:6789:abcd
              int port         server的端口信息
              tcp_client_config_t config  client的配置信息, 一般通过tcp_client_default_config
                                          生成, 并由用户自定义修改后确定
返回值描述:   1 函数处理成功
              0 函数处理失败, 失败原因参考日志, 其中记录了libuv失败的具体原因
备注: 该函数为异步连接函数, 返回成功并不等于TCP连接建立成功, 只是说明TCP连接尝试
      启动成功, TCP连接的成功或失败结果信息需要等待tcp_client_config_t中注册的
      连接成功/失败回调函数获取
*/
int tcp_client_connect(tcp_client_t* xclient, 
                       uv_loop_t* loop, 
                       const char* ip, 
                       int port, 
                       tcp_client_config_t config);


/*
函数功能描述: TCP Client 发送数据函数
参数描述:     tcp_client_t* xclient  用户定义的tcp_client_t指针
              void* data   用户待发送的数据首地址, 该地址必须为用户通过
              malloc申请的堆内存, 不能是临时的变量内存; 用户在调用tcp_client_send
              后不能再对此块内存做任何处理, 内存释放由本框架负责完成;
              unsigned int size      用户待发送的数据字节数量
返回值描述:   1 函数处理成功
              0 函数处理失败, 失败原因参考日志, 其中记录了失败的具体原因
备注: 该函数为异步数据发送函数, 返回成功并不等于数据发送成功;
*/
int tcp_client_send(tcp_client_t* xclient, void* data, unsigned int size);


/*
函数功能描述: TCP Client 临时主动断开连接函数
参数描述:     tcp_client_t* xclient  用户定义的tcp_client_t指针
返回值描述:   1 函数处理成功
              0 函数处理失败, 失败原因参考日志, 其中记录了失败的具体原因
备注: 如果用户注册了心跳回调函数并设置了心跳周期, 该函数会断开与server的连接;
      但是在心跳周期时间到了以后, 自动重新建立连接; 
      如果用户希望彻底断开连接, 请使用tcp_client_shutdown函数
*/
int tcp_client_disconnect(tcp_client_t* xclient);


/*
函数功能描述: TCP Client 主动断开连接函数
参数描述:     tcp_client_t* xclient  用户定义的tcp_client_t指针
返回值描述:   1 函数处理成功
              0 函数处理失败, 失败原因参考日志, 其中记录了失败的具体原因
*/
int tcp_client_shutdown(tcp_client_t* xclient);




#ifdef __cplusplus
} // extern "C"
#endif

#endif /* TCP_SERVER_H */

