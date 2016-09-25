/* 
 * File:   udp_server.h
 * Author: sundq
 *
 * Created on 2016年1月25日, 下午12:45
 */

#ifndef UDP_SERVER_H
#define	UDP_SERVER_H

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

typedef struct udp_socket_s udp_socket_t;


/* 事件回调 */
typedef void (*udp_handle_on_data_func_t)(udp_socket_t *conn, const struct sockaddr *peer, void *data, size_t len); //udp数据读取回调函数
typedef void (*udp_handle_on_send_func_t)(udp_socket_t *conn, const struct sockaddr *peer, void *data, size_t len, int status); //udp发送数据回调函数
typedef void (*udp_handle_on_error_func_t)(const char *errmsg, udp_socket_t *conn); //错误回调函数


udp_socket_t *udp_handle_server_create(const char *server_addr,
                              int server_port,
                              int mode,
                              udp_handle_on_data_func_t on_data,
                              udp_handle_on_send_func_t on_send,
                              udp_handle_on_error_func_t on_error);
void udp_handle_run(udp_socket_t *udp_handle);
//发送数据到特定地址，地址用struct sockaddr 表示
void udp_send_data(udp_socket_t *handle, char *data, size_t size, const struct sockaddr *addr);
//发送数据到特定ip和端口 如127.0.0. 8000
void udp_send_data_ip(udp_socket_t *udp_handle, char *data, size_t size, const char *ip, unsigned short port);
//打开广播开关
int udp_set_broadcast(udp_socket_t *udp_handle, int on); 


//udp实例结构 

struct udp_socket_s
{
    uv_loop_t* uvloop;
    uv_udp_t udp; //udp的uv实例
    size_t bytes_recv;
    size_t bytes_send;
    struct tms tmsstart;//启动的时间， 用户计算性能参数
    uv_timer_t timer; //打印性能参数的计时器
    void* data; //公共数据

    //各个回调函数
    udp_handle_on_data_func_t on_data;
    udp_handle_on_send_func_t on_send;
    udp_handle_on_error_func_t on_error;
};

#ifdef	__cplusplus
}
#endif

#endif	/* UDP_SERVER_H */

