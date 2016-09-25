/* 
 * File:   tcp_server.h
 * Author: sundq
 *
 * Created on 2016年1月23日, 下午2:52
 */

#ifndef TCP_SERVER_H
#define	TCP_SERVER_H

#ifdef	__cplusplus
extern "C"
{
#endif

#include <uv.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

typedef struct tcp_client_s tcp_client_t;
typedef struct tcp_server_real tcp_server_t;
typedef struct tcp_send_data_cb_s tcp_send_data_cb_t;

/* 服务端事件回调 */
typedef void (*tcp_server_on_open_func_t)(tcp_client_t *conn); //tcp链接创建的回调函数
typedef void (*tcp_server_on_close_func_t)(tcp_client_t *conn); // tcp链接关闭的回调函数
typedef void (*tcp_server_on_data_func_t)(tcp_client_t *conn, void *data, size_t len); //tcp数据读取回调函数
typedef void (*tcp_server_on_error_func_t)(const char *errmsg, tcp_client_t *conn); //服务错误回调函数
//异步发送数据回到函数, status 为0表示发送成功，否则发送失败
typedef void (*tcp_server_on_data_send_func_t)(tcp_client_t *conn, int status, void *data, size_t len);

struct tcp_server_real
{
    char name[32]; // tcp服务器名字
    const char *server_addr;
    int port;
    uv_loop_t* uvloop;
    uv_tcp_t server; //服务句柄
    int conn_backlog; // used by uv_listen()
    int max_connections; // 最大连接数
    int conn_count; //当前链接总数
    size_t bytes_recv;
    size_t bytes_send;
    struct tms tmsstart;//启动的时间， 用户计算性能参数
    uv_timer_t timer; //打印性能参数的计时器

    // callbacks
    tcp_server_on_open_func_t on_conn_open;
    tcp_server_on_error_func_t on_conn_error;
    tcp_server_on_close_func_t on_conn_close;
    tcp_server_on_data_func_t on_data;
    tcp_server_on_data_send_func_t on_send;
};

struct tcp_client_s
{
    uv_loop_t* uvloop; //is NULL IN SERVER SIDE
    uv_tcp_t* client;
    tcp_server_t* server; //is NULL IN CLIENT SIDE 
    uv_connect_t* connect_req; // is NULL IN SERVER SIDE
    uv_async_t async_handle;
    int is_closed; //0 未关闭 1 关闭
    void *data; //public data

    // callbacks
    tcp_server_on_open_func_t on_conn_open;
    tcp_server_on_error_func_t on_conn_error;
    tcp_server_on_close_func_t on_conn_close;
    tcp_server_on_data_func_t on_data;
    tcp_server_on_data_send_func_t on_send;
};

//异步发送数据回调使用
struct tcp_send_data_cb_s
{
	tcp_client_t *tcp_client;
	uv_buf_t send_data; //发送的数据
};

tcp_server_t *tcp_server_create(const char *server_addr,
                              int server_port,
                              int backlog,
                              int mode,
                              tcp_server_on_open_func_t,
                              tcp_server_on_close_func_t,
                              tcp_server_on_error_func_t,
                              tcp_server_on_data_func_t,
                              tcp_server_on_data_send_func_t);
/* 设置最大连接数， 如果是0表示不受限制， 因为超过此参数导致拒绝新客户连接时，请在日志中输出日志 */
void tcp_server_set_max_conn(tcp_server_t *, int);
/*停止tcp服务*/
void tcp_server_close(tcp_server_t *);
/*发送数据 异步接口*/
void tcp_server_send_data(tcp_client_t *client, char *data, size_t size);
/*关闭链接*/
void tcp_server_close_client(tcp_client_t *client);
/* 获取客户端地址 */
char *tcp_server_get_client_addr(tcp_client_t *client, char *dst, size_t len);

void tcp_server_run(tcp_server_t *tcp_server);
#ifdef	__cplusplus
}
#endif

#endif	/* TCP_SERVER_H */

