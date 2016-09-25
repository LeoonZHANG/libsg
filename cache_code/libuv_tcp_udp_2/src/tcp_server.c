/*
 * tcp_server.c
 * High concurrency TCP server library based on libuv.
 */

#include <stdio.h>
#include <stdint.h>
#include <memory.h>
#include <time.h>
#include <assert.h>
#include "tcp_common.h"
#include "tcp_server.h"
#include "utils/linkhash.h"
 

 
 /* Server 私有管理结构 */
 typedef struct tcp_server_private_s {
     uv_timer_t heartbeat_timer;    /* 周期定时器句柄 */
     unsigned int heartbeat_index;  /* 周期定时器触发次数 */
     struct lh_table* conns;        /* server的连接管理hash表 */
 } tcp_server_private_t;

/* 快速引用server private去的宏定义 */ 
#define _TCP_S_PRIVATE(x)  ((tcp_server_private_t*)(&(x)->privates))
 
 static void tcp__on_connection(uv_stream_t* uvserver, int status);
 static void _uv_disconnect_client(uv_stream_t* uvclient);
 static void _uv_after_close_connection(uv_handle_t* handle);
 
 tcp_server_config_t tcp_server_default_config(tcp_server_t* xserver) {
     tcp_server_config_t config = { 0 };
     snprintf(config.name, sizeof(config.name), "tserver-%p", xserver);
     config.conn_count = 1024;
     config.conn_backlog = 128;
     config.conn_extra_size = 0;
     config.conn_timeout_seconds = 180.0;
     config.heartbeat_interval_seconds = 60.0;
     config.log_out = stdout;
     config.log_err = stderr;
     return config;
 }
 
 static void _tcp_check_timeout_clients(tcp_server_t* xserver);

 /* 定时器超时处理函数 */
 static void _uv_on_heartbeat_timer(uv_timer_t* handle) {
     tcp_server_t* xserver = (tcp_server_t*) handle->data;
     assert(xserver);

     /* 每超时一次heartbeat_index增1 */
     unsigned int index = ++(_TCP_S_PRIVATE(xserver)->heartbeat_index);
     if(xserver->config.log_out)
         fprintf(xserver->config.log_out, "[tcp-server] %s on heartbeat (index %u)\n", xserver->config.name, index);

     /* 如果用户定义了心跳定时回调, 则调用 */
     if(xserver->config.on_heartbeat)
         xserver->config.on_heartbeat(xserver, index);
         
     /* 遍历所有连接检查连接是否存在超时 */ 
     if(xserver->config.conn_timeout_seconds > 0)
         _tcp_check_timeout_clients(xserver);
 }
 
 int tcp_server_start(tcp_server_t* xserver, uv_loop_t* loop, const char* ip, int port, tcp_server_config_t config) {
     assert(xserver && loop && ip);
     xserver->uvloop = loop;
     memcpy(&xserver->config, &config, sizeof(tcp_server_config_t));

     /* 启动时先创建server连接管理hash表 */
     _TCP_S_PRIVATE(xserver)->conns = lh_kptr_table_new(config.conn_count, "clients connection table", NULL);
 
      /* 启动心跳周期定时器 */
     int timeout = (int)(config.heartbeat_interval_seconds * 1000); // in milliseconds
     uv_timer_init(loop, &_TCP_S_PRIVATE(xserver)->heartbeat_timer);
     _TCP_S_PRIVATE(xserver)->heartbeat_timer.data = xserver;
     _TCP_S_PRIVATE(xserver)->heartbeat_index = 0;
     if(timeout > 0)
         uv_timer_start(&_TCP_S_PRIVATE(xserver)->heartbeat_timer, _uv_on_heartbeat_timer, timeout, timeout);
 
     /* 初始化tcp句柄*/
     uv_tcp_init(loop, &xserver->uvserver);
     xserver->uvserver.data = xserver;
     if(strchr(ip, ':')) {
         struct sockaddr_in6 addr;
         uv_ip6_addr(ip, port, &addr);
         uv_tcp_bind(&xserver->uvserver, (const struct sockaddr*) &addr, 0);
     } else {
         struct sockaddr_in addr;
         uv_ip4_addr(ip, port, &addr);
         /*  绑定侦听端口  */
	     uv_tcp_bind(&xserver->uvserver, (const struct sockaddr*) &addr, 0); 
     }

     /* 启动监听 */
     int ret = uv_listen((uv_stream_t*)&xserver->uvserver, config.conn_backlog, tcp__on_connection);
     if(ret >= 0 && config.log_out) {
         char timestr[32]; time_t t; time(&t);
         strftime(timestr, sizeof(timestr), "[%Y-%m-%d %X]", localtime(&t)); // C99 only: %F = %Y-%m-%d
         fprintf(config.log_out, "[tcp-server] %s %s listening on %s:%d ...\n", timestr, xserver->config.name, ip, port);
     }
     if(ret < 0 && config.log_err)
         fprintf(config.log_err, "\n!!! [tcp-server] %s listen on %s:%d failed: %s\n", xserver->config.name, ip, port, uv_strerror(ret));
 
     return (ret >= 0);
 }
 
 int tcp_server_shutdown(tcp_server_t* xserver) {
     /* 停止周期定时器 */
     if (xserver->config.heartbeat_interval_seconds > 0)
     {
        uv_timer_stop(&_TCP_S_PRIVATE(xserver)->heartbeat_timer);
     }   

     /* 关闭周期定时器句柄 */
     uv_close((uv_handle_t*)&_TCP_S_PRIVATE(xserver)->heartbeat_timer, NULL);

     /* 释放连接管理hash表 */
     lh_table_free(_TCP_S_PRIVATE(xserver)->conns);

     /* 关闭server句柄 */
     uv_close((uv_handle_t*)&xserver->uvserver, NULL);
     return 0;
 }
 
 void tcp_server_conn_ref(tcp_server_conn_t* conn, int ref) {
     assert(ref == 1 || ref == -1);

     /* 先对信号量上锁, 保证对refcount操作原子化 */
     uv_mutex_lock(&conn->refmutex);
     conn->refcount += ref; // +1 or -1
     if(conn->refcount == 0) {
         /* 如果连接引用计数为0, 则该连接需要进行关闭
            释放相关连接资源 */
         uv_mutex_unlock(&conn->refmutex);
         uv_mutex_destroy(&conn->refmutex);
         free(conn);
         return;
     }
     uv_mutex_unlock(&conn->refmutex);
 }
 
 int tcp_server_conn_send(tcp_server_conn_t* conn, void* data, unsigned int size) {

     tcp_server_t* xserver = conn->xserver;
     assert(xserver);
     // 当连接上发送过数据后, 记录该连接通信时间
     conn->last_comm_time = uv_now(xserver->uvloop);
     
     return cmm_send_to_stream((uv_stream_t*)&conn->uvclient, data, size);
 }
 
 static void tcp__on_read(uv_stream_t* uvclient, ssize_t nread, const uv_buf_t* buf) {
     tcp_server_conn_t* conn = (tcp_server_conn_t*) uvclient->data;
     assert(conn);
     tcp_server_t* xserver = conn->xserver;
     assert(xserver);
     if(nread > 0) {
         // 当连接上读到数据后, 记录该连接通信时间
         conn->last_comm_time = uv_now(xserver->uvloop);

         /* 调用用户的读取回调处理函数 */
         if(xserver->config.on_recv)
             xserver->config.on_recv(xserver, conn, buf->base, nread);
     } else if(nread < 0) {
         /* 没有数据可读, 说明当前连接已经异常 */
         if(xserver->config.log_err)
             fprintf(xserver->config.log_err, "\n!!! [tcp-server] %s on recv error: %s\n", xserver->config.name, uv_strerror(nread));
         _uv_disconnect_client(uvclient);
     }
     free(buf->base);
 }

 /* 连接关闭后回调处理, 用于释放连接相关资源 */
 static void _uv_after_close_connection(uv_handle_t* handle) {
     tcp_server_conn_t* conn = (tcp_server_conn_t*) handle->data;
     assert(conn && conn->xserver);
     tcp_server_t* xserver = conn->xserver;

     /* 调用用户的连接关闭回调处理 */
     if(xserver->config.on_conn_close)
         xserver->config.on_conn_close(xserver, conn);

     /* 从连接管理hash表中删除连接 */    
     int n = lh_table_delete(_TCP_S_PRIVATE(xserver)->conns, (const void*)conn);
     assert(n == 0); //delete success
     tcp_server_conn_ref(conn, -1); // call on_conn_close() inside here? in non-main-thread?
 }

 /* 连接未建立成功的连接关闭后回调处理, 
    与正常的_uv_after_close_connection相比, 区别是:
    1) 不需要从内部连接管理hash表中删除连接;
    2) 不需要调用用户注册的连接关闭函数, 因为连接没有成功过
 */
 static void _uv_after_close_connection_on_conn_failed(uv_handle_t* handle) {
     tcp_server_conn_t* conn = (tcp_server_conn_t*) handle->data;
     assert(conn && conn->xserver);
     tcp_server_t* xserver = conn->xserver;

     tcp_server_conn_ref(conn, -1); // call on_conn_close() inside here? in non-main-thread?
 } 

 
 
 // defines in common.c
 void cmm__on_alloc_buf(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);

 /* tcp连接建立回调 */
 static void tcp__on_connection(uv_stream_t* uvserver, int status) {
     tcp_server_t* xserver = (tcp_server_t*) uvserver->data;
     assert(xserver);

     /* status为0时, 连接状态正常 */
     if(status == 0) {
         if(xserver->config.log_out)
             fprintf(xserver->config.log_out, "[tcp-server] %s on connection\n", xserver->config.name);
         assert(uvserver == (uv_stream_t*) &xserver->uvserver);
         assert(xserver->config.conn_extra_size >= 0);
 
         /* 创建连接句柄资源, 并进行成员赋值和初始化 */
         tcp_server_conn_t* conn = (tcp_server_conn_t*) calloc(1, sizeof(tcp_server_conn_t) + xserver->config.conn_extra_size);
         if(xserver->config.conn_extra_size > 0)
             conn->extra = (void*)(conn + 1);
         conn->xserver = xserver;
         conn->uvclient.data = conn;
         conn->last_comm_time = 0;
         conn->refcount = 1;
         uv_mutex_init(&conn->refmutex);
 


         /* 初始化tcp句柄 */
         uv_tcp_init(xserver->uvloop, &(conn->uvclient));
         if(uv_accept(uvserver, (uv_stream_t*) &(conn->uvclient)) == 0) {
             /* 连接建立成功, 检查当前连接数量是否超出server配置连接数量 */
             if (_TCP_S_PRIVATE(xserver)->conns->count >= xserver->config.conn_count)
             {
                 if(xserver->config.log_out)
                     fprintf(xserver->config.log_out, "[tcp-server] %s reach the connection %d limit.\n", 
                                xserver->config.name, xserver->config.conn_count);

                 /* 关闭前面当前连接 */               
                 uv_close((uv_handle_t*) &conn->uvclient, _uv_after_close_connection_on_conn_failed); 
                 return;
             }

             /* 连接未超出server配置最大数量, 将连接插入连接管理hash表 */
             assert(lh_table_lookup_entry(_TCP_S_PRIVATE(xserver)->conns, conn) == NULL);
             lh_table_insert(_TCP_S_PRIVATE(xserver)->conns, conn, (const void*)conn);

             /* accpet连接成功, 先记录连接时间, 并调用用户注册的连接成功回调 */
             conn->last_comm_time = uv_now(xserver->uvloop);
             if(xserver->config.on_conn_ok)
                 xserver->config.on_conn_ok(xserver, conn);

             /* 启动连接读回调 */
             uv_read_start((uv_stream_t*) &conn->uvclient, cmm__on_alloc_buf, tcp__on_read);
         } else {
             /* accpet失败, 调用用户注册的连接失败回调, 并关闭tcp句柄 */
             if(xserver->config.on_conn_fail) {
                 xserver->config.on_conn_fail(conn->xserver, conn);
             }

             /* 关闭前面当前连接 */ 
             uv_close((uv_handle_t*) &conn->uvclient, _uv_after_close_connection_on_conn_failed);
         }
     } else {
         /* 连接异常 */
         if(xserver->config.log_err)
             fprintf(xserver->config.log_err, "\n!!! [tcp-server] %s on connection error: %s\n", xserver->config.name, uv_strerror(status));
     }
 }

 /* tcp server 关闭一个连接  */
 static void _uv_disconnect_client(uv_stream_t* uvclient) {
     tcp_server_conn_t* conn = (tcp_server_conn_t*) uvclient->data;
     assert(conn && ((uv_stream_t*)&conn->uvclient == uvclient));

     /* 先停止句柄上的读事件回调 */
     uv_read_stop(uvclient);
     assert(conn->xserver);

     /* 调用用户注册的连接关闭前回调处理函数 */
     if(conn->xserver->config.on_conn_closing)
         conn->xserver->config.on_conn_closing(conn->xserver, conn);

     /* 关闭tcp句柄 */    
     uv_close((uv_handle_t*)uvclient, _uv_after_close_connection);
 }
 
 /* 遍历所有的tcp连接, 检查连接是否超时 */
 static void _tcp_check_timeout_clients(tcp_server_t* xserver) {
     int conn_timeout = (int) (xserver->config.conn_timeout_seconds * 1000); // in milliseconds

     /* 如果配置的超时时间不大于0, 则取消连接超时检查 */
     if(conn_timeout <= 0)
         return;
         
     struct lh_entry *e, *tmp;

     /* 循环遍历当前所有连接 */
     lh_foreach_safe(_TCP_S_PRIVATE(xserver)->conns, e, tmp) {
         tcp_server_conn_t* conn = (tcp_server_conn_t*) e->k;

         /* 检查当前时间与上次通信时间的差别  */
         if(uv_now(xserver->uvloop) - conn->last_comm_time > conn_timeout) {
             /* 已经超时, 则关闭该连接 */
             if(xserver->config.log_out)
                 fprintf(xserver->config.log_out, "[tcp-server] %s close connection %p for its long time silence\n",
                         xserver->config.name, &conn->uvclient);
             _uv_disconnect_client((uv_stream_t*) &conn->uvclient);
         } else {
             /* 继续遍历  */
             continue; 
         }
     }
 }
 
 int tcp_server_iter_conns(tcp_server_t* xserver, TCP_S_ON_ITER_CONN on_iter_conn, void* userdata) {
     /* 如果用户未定义遍历回调函数, 则直接返回当前连接数量 */
     if(on_iter_conn) {
         struct lh_entry *e, *tmp;
         /* 遍历当前所有的节点, 对每个节点执行用户定义的回调函数 */
         lh_foreach_safe(_TCP_S_PRIVATE(xserver)->conns, e, tmp) {
             on_iter_conn(xserver, (tcp_server_conn_t*) e->k, userdata);
         }
     }
     return _TCP_S_PRIVATE(xserver)->conns->count;
 }

