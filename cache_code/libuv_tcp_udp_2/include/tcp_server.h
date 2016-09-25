/*
 * tcp_server.h
 * High concurrency tcp server library based on libuv.
 */

#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__
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
/* 本头文件定义tcp server 相关的数据结构以及接口函数 */ 

typedef struct tcp_server_s tcp_server_t;
typedef struct tcp_server_conn_s tcp_server_conn_t;

/*tcp server 的回调处理函数接口定义 */

/* 
功能定义: TCP server 成功建立连接时回调处理函数 
是否必须定义: 可选定义, 用户未定义回调时连接将按照成功继续处理
参数解释: 
tcp_server_t* server  tcp server句柄, 具体请参考tcp_server_t内容定义
tcp_server_conn_t* conn  tcp 连接句柄, 具体请参考tcp_server_conn_t内容定义

*/
typedef void (*TCP_S_ON_CONN_OK)        (tcp_server_t* xserver, tcp_server_conn_t* conn);

/* 
功能定义: TCP server accept连接失败回调处理函数 
是否必须定义: 可选定义, 用户未定义回调时连接将按照连接失败继续处理
参数解释: 
tcp_server_t* server  tcp server句柄, 具体请参考tcp_server_t内容定义
tcp_server_conn_t* conn  tcp 连接句柄, 具体请参考tcp_server_conn_t内容定义
*/
typedef void (*TCP_S_ON_CONN_FAIL)      (tcp_server_t* xserver, tcp_server_conn_t* conn);

/* 
功能定义: TCP server 连接关闭前的回调处理函数 
是否必须定义: 可选定义, 用户未定义回调时连接将按照连接继续关闭处理
参数解释: 
tcp_server_t* server  tcp server句柄, 具体请参考tcp_server_t内容定义
tcp_server_conn_t* conn  tcp 连接句柄, 具体请参考tcp_server_conn_t内容定义
*/
typedef void (*TCP_S_ON_CONN_CLOSING)   (tcp_server_t* xserver, tcp_server_conn_t* conn);

/* 
功能定义: TCP server 连接关闭完成后的回调处理函数 
是否必须定义: 可选定义
参数解释: 
tcp_server_t* server  tcp server句柄, 具体请参考tcp_server_t内容定义
tcp_server_conn_t* conn  tcp 连接句柄, 具体请参考tcp_server_conn_t内容定义
*/
typedef void (*TCP_S_ON_CONN_CLOSE)     (tcp_server_t* xserver, tcp_server_conn_t* conn);


/* 
功能定义: TCP server  收到数据时回调处理函数 
是否必须定义: 可选定义, 如果用户未定义该回调, 所有收到的数据将会被丢弃;
              如果用户定义了该回调函数, 可以通过函数的输入参数获取收到的数据
参数解释: 
tcp_server_t* server  tcp server句柄, 具体请参考tcp_server_t内容定义
tcp_server_conn_t* conn  tcp 连接句柄, 具体请参考tcp_server_conn_t内容定义
void* data : 收到的数据流指针首地址, 该字节流长度由datalen指定
ssize_t datalen: 收到的数据流的字节长度
*/
typedef void (*TCP_S_ON_RECV)           (tcp_server_t* xserver, tcp_server_conn_t* conn, void* data, ssize_t datalen);

/* 
功能定义: TCP server 心跳周期时间到达时的回调处理函数
该定时器从用户调用tcp_server_start开始启动, 心跳周期时长由tcp_server_config_t.heartbeat_interval_seconds
决定; 
心跳功能为上层应用提供了一个简单的定时器, 用户可以在该函数中执行需要周期进行的逻辑处理; 
组件内部使用心跳来周期性遍历检查当前server已经建立的所有连接中是否存在有长时间未通信的
连接, 如果存在则主动关闭连接;
是否必须定义: 可选定义
参数解释: 
tcp_server_t* server  tcp server句柄, 具体请参考tcp_server_t内容定义
unsigned int index  心跳周期回调函数执行次数, 从用户调用tcp_server_start函数开始计数, 
                    初始值为0, 调用一次心跳周期回调函数该值增加1;                    
*/
typedef void (*TCP_S_ON_HEARTBEAT)      (tcp_server_t* xserver, unsigned int index);

typedef struct tcp_server_config_s {
    char name[32];          /* TCP server名字 */
    int conn_count;         /* server支持建立的最大连接数量, 默认为1024条 */
    int conn_backlog;       /* server 底层socket能缓存的最大待接收连接请求数量
                               用于 uv_listen(), 默认值128 */
    int conn_extra_size;    /* tcp_server_conn_t.extra 连接结构扩展数据区的大小
                               默认值为0 */
    float conn_timeout_seconds; /* 该参数用来指定连接的最长无通信时间, TCP server
                                  会定期检查建立的每条连接, 如果该连接已经超过conn_timeout_seconds
                                  未进行数据通信, 则TCP server会主动关闭该连接
                                  默认值180秒, 如果该值为0, 则TCP server不会检查连接是否超时 
                                  */
    float heartbeat_interval_seconds; /* TCP server 周期定时器时长
                                        默认为60秒*/
    // callbacks
    TCP_S_ON_CONN_OK        on_conn_ok;     /* 连接创建成功回调, 可选 */
    TCP_S_ON_CONN_FAIL      on_conn_fail;   /* 连接创建失败回调, 可选 */
    TCP_S_ON_CONN_CLOSING   on_conn_closing;/* 连接关闭前回调, 可选 */
    TCP_S_ON_CONN_CLOSE     on_conn_close;  /* 连接关闭完成后回调, 可选 */
    TCP_S_ON_HEARTBEAT      on_heartbeat;   /* 周期心跳回调函数, 可选 */
    TCP_S_ON_RECV           on_recv;        /* SERVER收到数据回调, 可选 */
    // logs
    FILE* log_out;
    FILE* log_err;
} tcp_server_config_t;

/* TCP server实例句柄数据结构, 每个TCP server对应一个tcp_server_s */
struct tcp_server_s {
    uv_loop_t* uvloop;          /* 该server句柄对应的uvloop句柄 */  
    uv_tcp_t   uvserver;        /* 该server句柄对应的uvserver句柄 */  
    tcp_server_config_t config; /* 该server句柄对应的config */ 
    unsigned char privates[136]; /* 该server句柄对应的内部管理数据 */ 
    void* data;                 /* 该server句柄对应的应用扩展数据区 */
};
typedef struct tcp_server_s tcp_server_t;

/* TCP server 连接实例句柄数据结构, 每个TCP 连接对应一个tcp_server_conn_s */
typedef struct tcp_server_conn_s {
    tcp_server_t* xserver;      /* 连接句柄 对应的server句柄 */
    uv_tcp_t uvclient;          /* 连接句柄 对应的client句柄 */
    uint64_t last_comm_time;    /* 连接最后一次通信的时间, 用于检查连接是否超时 */
    int refcount;               /* 连接被引用次数, 用户层不要直接修改该参数, 而应该
                                   通过调用tcp_server_conn_ref接口访问 */
    uv_mutex_t refmutex;        /* refcount的操作保护信号量 */ 
    void* extra;                /* 连接句柄 中的用户扩展数据区, 大小由 
                                    tcp_server_config_t.conn_extra_size 决定, 
                                    默认为NULL */               
} tcp_server_conn_t;

/*
函数功能描述: 生成TCP server的默认配置信息, 用户可用该函数先生成一个默认配置的
              tcp_server_config_t, 然后根据自己需要修改其中部分参数
参数描述:     tcp_server_t* xserver  用户定义的tcp_server_t指针
返回值描述:   返回一个填充默认值的tcp_server_config_t结构
*/
tcp_server_config_t tcp_server_default_config(tcp_server_t* xserver);



/*
函数功能描述: 启动TCP server
参数描述:     tcp_server_t* xserver  用户定义的tcp_server_t指针
              uv_loop_t* loop        用户定义的uv_loop_t指针, 通常调用libuv接口函数获得
              const char* ip         TCP server侦听的ip地址, 如果想侦听系统上所有的ip地址,
                                     侦听0.0.0.0
              int port               TCP server侦听的端口
              tcp_server_config_t config TCP server运行的配置和处理回调
返回值描述:   成功返回 1
              失败范围 0
*/
int tcp_server_start(tcp_server_t* xserver, 
                     uv_loop_t* loop, 
                     const char* ip, 
                     int port, 
                     tcp_server_config_t config);


/*
函数功能描述: 关闭TCP server
参数描述:     tcp_server_t* xserver  用户定义的tcp_server_t指针
返回值描述:   成功返回 1
              失败范围 0
*/
int tcp_server_shutdown(tcp_server_t* xserver);

/* 
功能定义: 用户调用tcp_server_iter_conns函数遍历TCP server 连接的遍历回调处理函数 
是否必须定义: 请配合tcp_server_iter_conns函数说明进行使用
参数解释: 
tcp_server_t* server  tcp server句柄, 具体请参考tcp_server_t内容定义
tcp_server_conn_t* conn  tcp 连接句柄, 具体请参考tcp_server_conn_t内容定义
void* userdata  用户输入的自定义数据区, 内容由用户自行定义
*/
typedef void (*TCP_S_ON_ITER_CONN)      (tcp_server_t* xserver, tcp_server_conn_t* conn, void* userdata);


/*
函数功能描述: 遍历TCP server当前建立的所有连接
参数描述:     tcp_server_t* xserver  用户定义的tcp_server_t指针
              TCP_S_ON_ITER_CONN on_iter_conn  用户定义的连接遍历回调函数, 该参数
                                            如果为NULL, 则本函数不进行遍历操作;
              void* userdata    用户扩展数据指针
返回值描述:   返回当前实际建立的连接数量
*/
int tcp_server_iter_conns(tcp_server_t* xserver, TCP_S_ON_ITER_CONN on_iter_conn, void* userdata);

/*
函数功能描述: TCP连接引用次数管理接口, 用于多个模块均持有同一个连接时
              比如有A/B/C/D四个模块均持有了同一个连接conn, 则该模块在持有该连接前
              调用tcp_server_conn_ref将引用计数增1, 该模块释放该连接时, 调用tcp_server_conn_ref
              将引用计数减1;
参数描述:     tcp_server_conn_t* conn   需要操作的连接指针
              int ref                   引用计数变化值, 该值必须为1或-1
返回值描述:   无
            
*/
void tcp_server_conn_ref(tcp_server_conn_t* conn, int ref);

/*
函数功能描述: TCP server 主动发送数据接口
参数描述:     tcp_server_conn_t* conn   需要操作的连接指针
              void* data   用户待发送的数据首地址, 该地址必须为用户通过
             malloc申请的堆内存, 不能是临时的变量内存; 用户在调用tcp_server_conn_send
             后不能再对此块内存做任何处理, 内存释放由本框架负责完成;
              unsigned int size      用户待发送的数据字节数量

返回值描述:  成功返回 1
             失败范围 0
备注: 该函数为异步数据发送函数, 返回成功并不等于数据发送成功;  

*/
int tcp_server_conn_send(tcp_server_conn_t* conn, void* data, unsigned int size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* TCP_SERVER_H */
