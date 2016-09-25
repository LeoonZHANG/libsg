/*
 * tcp.c
 * High concurrency tcp client library based on libuv.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <memory.h>
#include <assert.h>
#include "tcp_common.h"
#include "tcp.h"
#include "utils/linkhash.h"


/*
TCP client地址联合
*/
typedef union tcp_sockaddr_4_6_s{
    struct sockaddr_in  in4;            /* ipv4地址 */
    struct sockaddr_in6 in6;            /* ipv6地址 */
} tcp_sockaddr_4_6_t;

/*
TCP client 句柄私有信息数据结构, 保存不对外部暴露的信息
*/
typedef struct tcp_client_private_s {
    uv_connect_t conn;              /* 与server建立的连接信息*/ 
    tcp_sockaddr_4_6_t server_addr; /* server的ip和端口信息 */
    uv_timer_t heartbeat_timer;     /* 心跳定时器句柄 */
    unsigned int heartbeat_index;   /* 心跳周期超时计数器, 记录周期个数 */
    int connection_closed;          /* 记录client与server的连接是否关闭 */
    int retried_times;              /* 已经连续重试的次数 */
} tcp_client_private_t;

/* 获取client private结构宏, 快速获取tcp_client_private_t地址 */
#define TCP__C_PRIVATE(x)  ((tcp_client_private_t*)(&(x)->privates))

tcp_client_config_t tcp_client_default_config(tcp_client_t* xclient) {
    tcp_client_config_t config = { 0 };
    snprintf(config.name, sizeof(config.name), "xclient-%p", xclient);
    config.retry_times = 5;
    config.heartbeat_interval_seconds = 60.0;
    config.log_out = stdout;
    config.log_err = stderr;
    return config;
}

static int tcp__client_reconnect(tcp_client_t* xclient);

static void tcp__on_heartbeat_timer(uv_timer_t* handle) {
    tcp_client_t* xclient = (tcp_client_t*) handle->data;
    assert(xclient);

    /*
        心跳回调函数, 如果连接已经建立, 则调用用户注册的回调
                      如果连接未建立, 自动进行重连
    */
    if(xclient->uvserver) {
        unsigned int index = TCP__C_PRIVATE(xclient)->heartbeat_index++;
        if(xclient->config.log_out)
            fprintf(xclient->config.log_out, "[tcp-client] %s on heartbeat (index %u)\n", xclient->config.name, index);
        if(xclient->config.on_heartbeat)
            xclient->config.on_heartbeat(xclient, index);
    } else {
        if(TCP__C_PRIVATE(xclient)->connection_closed)
        {   
            /* 如果xclient->config.retry_times 配置为0, 则无限次重连 */
            if (xclient->config.retry_times <= 0)
            {
                tcp__client_reconnect(xclient);
            }
            else
            {
                /* 如果重连次数小于配置重连次数与, 则继续重新连接 */
                if (TCP__C_PRIVATE(xclient)->retried_times < xclient->config.retry_times)
                {
                    tcp__client_reconnect(xclient);
                }
                else
                {
                    fprintf(xclient->config.log_out, 
                            "[tcp-client] %s has retried %d times, stop trying\n", 
                            xclient->config.name, TCP__C_PRIVATE(xclient)->retried_times);
                    //停止libuv定时器
                    uv_timer_stop(&TCP__C_PRIVATE(xclient)->heartbeat_timer);
                }
            }
        }    
    }
}

int tcp_client_connect(tcp_client_t* xclient, uv_loop_t* loop, const char* ip, int port, tcp_client_config_t config) {
	assert(xclient && loop && ip);
	xclient->uvloop = loop;
    xclient->uvserver = NULL;

    /* 在连接前, 将私有区数据初始化 */
    TCP__C_PRIVATE(xclient)->connection_closed = 0;
    TCP__C_PRIVATE(xclient)->retried_times     = 0;
    memcpy(&xclient->config, &config, sizeof(tcp_client_config_t));
    if(strchr(ip, ':'))
        uv_ip6_addr(ip, port, (struct sockaddr_in6*) &TCP__C_PRIVATE(xclient)->server_addr);
    else
        uv_ip4_addr(ip, port, (struct sockaddr_in*) &TCP__C_PRIVATE(xclient)->server_addr);

    //发起连接尝试, 首次连接和重连公用tcp__client_reconnect函数
    int ret = tcp__client_reconnect(xclient);
    if (ret == 1)
    {
        /*
            如果连接操作发起成功, 启动心跳周期定时器
        */
        int timeout = (int)(config.heartbeat_interval_seconds * 1000); // in milliseconds
        TCP__C_PRIVATE(xclient)->heartbeat_index = 0;
        TCP__C_PRIVATE(xclient)->heartbeat_timer.data = xclient;  

        //初始化libuv定时器并启动
    	uv_timer_init(loop, &TCP__C_PRIVATE(xclient)->heartbeat_timer);
    	uv_timer_start(&TCP__C_PRIVATE(xclient)->heartbeat_timer, tcp__on_heartbeat_timer, timeout, timeout);
    }
	return ret;
}


int tcp_client_send(tcp_client_t* xclient, void* data, unsigned int size) {
    /* 判断连接是否建立成功, 建立成功则发送数据 */
	if (xclient->uvserver)
		return cmm_send_to_stream((uv_stream_t*)xclient->uvserver, data, size);
	else
		return 0;
}

static void _uv_on_connect(uv_connect_t* conn, int status);

static int tcp__client_reconnect(tcp_client_t* xclient) {
    xclient->uvserver = NULL;
    
	TCP__C_PRIVATE(xclient)->connection_closed = 0;
    TCP__C_PRIVATE(xclient)->conn.data = xclient;
    TCP__C_PRIVATE(xclient)->retried_times++;
    /* 初始化uvclient */
	uv_tcp_init(xclient->uvloop, &xclient->uvclient);

	/* 尝试连接server, 并设置连接回调处理 */
    int ret = uv_tcp_connect(&TCP__C_PRIVATE(xclient)->conn, &xclient->uvclient,
                             (const struct sockaddr*) &TCP__C_PRIVATE(xclient)->server_addr, _uv_on_connect);
    if(ret >= 0 && xclient->config.log_out)
        fprintf(xclient->config.log_out, "[tcp-client] %s connect to server ...\n", xclient->config.name);
    if(ret < 0 && xclient->config.log_err)
        fprintf(xclient->config.log_err, "\n!!! [tcp-client] %s connect failed: %s\n", xclient->config.name, uv_strerror(ret));
    return (ret >= 0 ? 1 : 0);
}

static void tcp__after_close_client(uv_handle_t* handle) {
    tcp_client_t* xclient = (tcp_client_t*) handle->data;
    assert(handle->data);

    /* 调用用户的连接关闭后回调函数, 并清理资源 */
    if(xclient->config.on_conn_close)
        xclient->config.on_conn_close(xclient);
    xclient->uvserver = NULL;
    TCP__C_PRIVATE(xclient)->connection_closed = 1;
}

static void _tcp_client_close(tcp_client_t* xclient) {
    if(xclient->config.log_out)
        fprintf(xclient->config.log_out, "[tcp-client] %s on close\n", xclient->config.name);
    /* client 主动发起连接关闭操作, 调用用户连接关闭前回调 */
    if(xclient->config.on_conn_closing)
        xclient->config.on_conn_closing(xclient);

    /* 关闭分配的uvclient handle资源 */    
    uv_close((uv_handle_t*) &xclient->uvclient, tcp__after_close_client);
}


static void tcp__on_client_read(uv_stream_t* uvserver, ssize_t nread, const uv_buf_t* buf) {
    tcp_client_t* xclient = (tcp_client_t*) uvserver->data;
    assert(xclient);

    /* client当前连接有数据可读, 先判断可读数据大小 */
	if(nread > 0) {
	    /* 大于0, 则直接调用用户注册的数据读取回调 */
        assert(xclient->uvserver == (uv_tcp_t*)uvserver);
        if(xclient->config.on_recv)
            xclient->config.on_recv(xclient, buf->base, nread);
        /* 当nread>0时, 该数据已经提供给用户, 此处需要释放掉buf指针 */
        free(buf->base);    
	} else if(nread < 0) {
	    /* 小于0, 说明当前连接已经异常, 直接停止读取回调, 并关闭连接
	       特别注意: 当 nread < 0时, buf并不会有缓存数据, 不需要进行内存释放 */

	    /* 停止libuv的读事件处理 */   
		uv_read_stop(uvserver);
		if(xclient->config.log_err)
            fprintf(xclient->config.log_err, "\n!!! [tcp-client] %s on recv error: %s\n", xclient->config.name, uv_strerror(nread));
		_tcp_client_close(xclient); // will try reconnect on next tcp__on_heartbeat_timer()
	}   
}

// 该函数的实现定义在 common.c 中
void cmm__on_alloc_buf(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);

static void _uv_on_connect(uv_connect_t* conn, int status) {
    tcp_client_t* xclient = (tcp_client_t*) conn->data;
    xclient->uvclient.data = xclient;

    /* 连接结果回调处理函数, 当status 为 0时, 连接建立成功, 否则连接建立失败 */
	if(status == 0) {
		if(xclient->config.log_out)
            fprintf(xclient->config.log_out, "[tcp-client] %s connect to server ok\n", xclient->config.name);
		assert(conn->handle == (uv_stream_t*) &xclient->uvclient);
		xclient->uvserver = (uv_tcp_t*) conn->handle;

        /* 连接成功, 将连续失败重连次数清零 */
        TCP__C_PRIVATE(xclient)->retried_times = 0;
        
		/* 调用用户注册的连接成功处理回调函数 */
        if(xclient->config.on_conn_ok)
            xclient->config.on_conn_ok(xclient);

        /* 启动libuv读事件处理, 并注册读回调 */    
		uv_read_start(conn->handle, cmm__on_alloc_buf, tcp__on_client_read);
	} else {
	    /* 连接建立失败 */
		xclient->uvserver = NULL;
		if(xclient->config.log_err)
            fprintf(xclient->config.log_err, "\n!!! [tcp-client] %s connect to server failed: %s\n", xclient->config.name, uv_strerror(status));

        /* 调用用户注册的连接失败处理回调函数 */
        if(xclient->config.on_conn_fail)
            xclient->config.on_conn_fail(xclient);

        /* todo: 确认此处不应该调用_tcp_client_close, 而应该直接关闭handle即可 */
		_tcp_client_close(xclient); // will try reconnect on next on_heartbeat
	}
}

int tcp_client_disconnect(tcp_client_t* xclient) {
	_tcp_client_close(xclient);
	return 1;
}

int tcp_client_shutdown(tcp_client_t* xclient) {
    /* shutdown操作首先停止心跳定时器 */
	uv_timer_stop(&TCP__C_PRIVATE(xclient)->heartbeat_timer);
	_tcp_client_close(xclient);
	return 1;
}

