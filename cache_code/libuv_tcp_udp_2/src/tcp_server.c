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
 

 
 /* Server ˽�й���ṹ */
 typedef struct tcp_server_private_s {
     uv_timer_t heartbeat_timer;    /* ���ڶ�ʱ����� */
     unsigned int heartbeat_index;  /* ���ڶ�ʱ���������� */
     struct lh_table* conns;        /* server�����ӹ���hash�� */
 } tcp_server_private_t;

/* ��������server privateȥ�ĺ궨�� */ 
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

 /* ��ʱ����ʱ������ */
 static void _uv_on_heartbeat_timer(uv_timer_t* handle) {
     tcp_server_t* xserver = (tcp_server_t*) handle->data;
     assert(xserver);

     /* ÿ��ʱһ��heartbeat_index��1 */
     unsigned int index = ++(_TCP_S_PRIVATE(xserver)->heartbeat_index);
     if(xserver->config.log_out)
         fprintf(xserver->config.log_out, "[tcp-server] %s on heartbeat (index %u)\n", xserver->config.name, index);

     /* ����û�������������ʱ�ص�, ����� */
     if(xserver->config.on_heartbeat)
         xserver->config.on_heartbeat(xserver, index);
         
     /* �����������Ӽ�������Ƿ���ڳ�ʱ */ 
     if(xserver->config.conn_timeout_seconds > 0)
         _tcp_check_timeout_clients(xserver);
 }
 
 int tcp_server_start(tcp_server_t* xserver, uv_loop_t* loop, const char* ip, int port, tcp_server_config_t config) {
     assert(xserver && loop && ip);
     xserver->uvloop = loop;
     memcpy(&xserver->config, &config, sizeof(tcp_server_config_t));

     /* ����ʱ�ȴ���server���ӹ���hash�� */
     _TCP_S_PRIVATE(xserver)->conns = lh_kptr_table_new(config.conn_count, "clients connection table", NULL);
 
      /* �����������ڶ�ʱ�� */
     int timeout = (int)(config.heartbeat_interval_seconds * 1000); // in milliseconds
     uv_timer_init(loop, &_TCP_S_PRIVATE(xserver)->heartbeat_timer);
     _TCP_S_PRIVATE(xserver)->heartbeat_timer.data = xserver;
     _TCP_S_PRIVATE(xserver)->heartbeat_index = 0;
     if(timeout > 0)
         uv_timer_start(&_TCP_S_PRIVATE(xserver)->heartbeat_timer, _uv_on_heartbeat_timer, timeout, timeout);
 
     /* ��ʼ��tcp���*/
     uv_tcp_init(loop, &xserver->uvserver);
     xserver->uvserver.data = xserver;
     if(strchr(ip, ':')) {
         struct sockaddr_in6 addr;
         uv_ip6_addr(ip, port, &addr);
         uv_tcp_bind(&xserver->uvserver, (const struct sockaddr*) &addr, 0);
     } else {
         struct sockaddr_in addr;
         uv_ip4_addr(ip, port, &addr);
         /*  �������˿�  */
	     uv_tcp_bind(&xserver->uvserver, (const struct sockaddr*) &addr, 0); 
     }

     /* �������� */
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
     /* ֹͣ���ڶ�ʱ�� */
     if (xserver->config.heartbeat_interval_seconds > 0)
     {
        uv_timer_stop(&_TCP_S_PRIVATE(xserver)->heartbeat_timer);
     }   

     /* �ر����ڶ�ʱ����� */
     uv_close((uv_handle_t*)&_TCP_S_PRIVATE(xserver)->heartbeat_timer, NULL);

     /* �ͷ����ӹ���hash�� */
     lh_table_free(_TCP_S_PRIVATE(xserver)->conns);

     /* �ر�server��� */
     uv_close((uv_handle_t*)&xserver->uvserver, NULL);
     return 0;
 }
 
 void tcp_server_conn_ref(tcp_server_conn_t* conn, int ref) {
     assert(ref == 1 || ref == -1);

     /* �ȶ��ź�������, ��֤��refcount����ԭ�ӻ� */
     uv_mutex_lock(&conn->refmutex);
     conn->refcount += ref; // +1 or -1
     if(conn->refcount == 0) {
         /* ����������ü���Ϊ0, ���������Ҫ���йر�
            �ͷ����������Դ */
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
     // �������Ϸ��͹����ݺ�, ��¼������ͨ��ʱ��
     conn->last_comm_time = uv_now(xserver->uvloop);
     
     return cmm_send_to_stream((uv_stream_t*)&conn->uvclient, data, size);
 }
 
 static void tcp__on_read(uv_stream_t* uvclient, ssize_t nread, const uv_buf_t* buf) {
     tcp_server_conn_t* conn = (tcp_server_conn_t*) uvclient->data;
     assert(conn);
     tcp_server_t* xserver = conn->xserver;
     assert(xserver);
     if(nread > 0) {
         // �������϶������ݺ�, ��¼������ͨ��ʱ��
         conn->last_comm_time = uv_now(xserver->uvloop);

         /* �����û��Ķ�ȡ�ص������� */
         if(xserver->config.on_recv)
             xserver->config.on_recv(xserver, conn, buf->base, nread);
     } else if(nread < 0) {
         /* û�����ݿɶ�, ˵����ǰ�����Ѿ��쳣 */
         if(xserver->config.log_err)
             fprintf(xserver->config.log_err, "\n!!! [tcp-server] %s on recv error: %s\n", xserver->config.name, uv_strerror(nread));
         _uv_disconnect_client(uvclient);
     }
     free(buf->base);
 }

 /* ���ӹرպ�ص�����, �����ͷ����������Դ */
 static void _uv_after_close_connection(uv_handle_t* handle) {
     tcp_server_conn_t* conn = (tcp_server_conn_t*) handle->data;
     assert(conn && conn->xserver);
     tcp_server_t* xserver = conn->xserver;

     /* �����û������ӹرջص����� */
     if(xserver->config.on_conn_close)
         xserver->config.on_conn_close(xserver, conn);

     /* �����ӹ���hash����ɾ������ */    
     int n = lh_table_delete(_TCP_S_PRIVATE(xserver)->conns, (const void*)conn);
     assert(n == 0); //delete success
     tcp_server_conn_ref(conn, -1); // call on_conn_close() inside here? in non-main-thread?
 }

 /* ����δ�����ɹ������ӹرպ�ص�����, 
    ��������_uv_after_close_connection���, ������:
    1) ����Ҫ���ڲ����ӹ���hash����ɾ������;
    2) ����Ҫ�����û�ע������ӹرպ���, ��Ϊ����û�гɹ���
 */
 static void _uv_after_close_connection_on_conn_failed(uv_handle_t* handle) {
     tcp_server_conn_t* conn = (tcp_server_conn_t*) handle->data;
     assert(conn && conn->xserver);
     tcp_server_t* xserver = conn->xserver;

     tcp_server_conn_ref(conn, -1); // call on_conn_close() inside here? in non-main-thread?
 } 

 
 
 // defines in common.c
 void cmm__on_alloc_buf(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);

 /* tcp���ӽ����ص� */
 static void tcp__on_connection(uv_stream_t* uvserver, int status) {
     tcp_server_t* xserver = (tcp_server_t*) uvserver->data;
     assert(xserver);

     /* statusΪ0ʱ, ����״̬���� */
     if(status == 0) {
         if(xserver->config.log_out)
             fprintf(xserver->config.log_out, "[tcp-server] %s on connection\n", xserver->config.name);
         assert(uvserver == (uv_stream_t*) &xserver->uvserver);
         assert(xserver->config.conn_extra_size >= 0);
 
         /* �������Ӿ����Դ, �����г�Ա��ֵ�ͳ�ʼ�� */
         tcp_server_conn_t* conn = (tcp_server_conn_t*) calloc(1, sizeof(tcp_server_conn_t) + xserver->config.conn_extra_size);
         if(xserver->config.conn_extra_size > 0)
             conn->extra = (void*)(conn + 1);
         conn->xserver = xserver;
         conn->uvclient.data = conn;
         conn->last_comm_time = 0;
         conn->refcount = 1;
         uv_mutex_init(&conn->refmutex);
 


         /* ��ʼ��tcp��� */
         uv_tcp_init(xserver->uvloop, &(conn->uvclient));
         if(uv_accept(uvserver, (uv_stream_t*) &(conn->uvclient)) == 0) {
             /* ���ӽ����ɹ�, ��鵱ǰ���������Ƿ񳬳�server������������ */
             if (_TCP_S_PRIVATE(xserver)->conns->count >= xserver->config.conn_count)
             {
                 if(xserver->config.log_out)
                     fprintf(xserver->config.log_out, "[tcp-server] %s reach the connection %d limit.\n", 
                                xserver->config.name, xserver->config.conn_count);

                 /* �ر�ǰ�浱ǰ���� */               
                 uv_close((uv_handle_t*) &conn->uvclient, _uv_after_close_connection_on_conn_failed); 
                 return;
             }

             /* ����δ����server�����������, �����Ӳ������ӹ���hash�� */
             assert(lh_table_lookup_entry(_TCP_S_PRIVATE(xserver)->conns, conn) == NULL);
             lh_table_insert(_TCP_S_PRIVATE(xserver)->conns, conn, (const void*)conn);

             /* accpet���ӳɹ�, �ȼ�¼����ʱ��, �������û�ע������ӳɹ��ص� */
             conn->last_comm_time = uv_now(xserver->uvloop);
             if(xserver->config.on_conn_ok)
                 xserver->config.on_conn_ok(xserver, conn);

             /* �������Ӷ��ص� */
             uv_read_start((uv_stream_t*) &conn->uvclient, cmm__on_alloc_buf, tcp__on_read);
         } else {
             /* accpetʧ��, �����û�ע�������ʧ�ܻص�, ���ر�tcp��� */
             if(xserver->config.on_conn_fail) {
                 xserver->config.on_conn_fail(conn->xserver, conn);
             }

             /* �ر�ǰ�浱ǰ���� */ 
             uv_close((uv_handle_t*) &conn->uvclient, _uv_after_close_connection_on_conn_failed);
         }
     } else {
         /* �����쳣 */
         if(xserver->config.log_err)
             fprintf(xserver->config.log_err, "\n!!! [tcp-server] %s on connection error: %s\n", xserver->config.name, uv_strerror(status));
     }
 }

 /* tcp server �ر�һ������  */
 static void _uv_disconnect_client(uv_stream_t* uvclient) {
     tcp_server_conn_t* conn = (tcp_server_conn_t*) uvclient->data;
     assert(conn && ((uv_stream_t*)&conn->uvclient == uvclient));

     /* ��ֹͣ����ϵĶ��¼��ص� */
     uv_read_stop(uvclient);
     assert(conn->xserver);

     /* �����û�ע������ӹر�ǰ�ص������� */
     if(conn->xserver->config.on_conn_closing)
         conn->xserver->config.on_conn_closing(conn->xserver, conn);

     /* �ر�tcp��� */    
     uv_close((uv_handle_t*)uvclient, _uv_after_close_connection);
 }
 
 /* �������е�tcp����, ��������Ƿ�ʱ */
 static void _tcp_check_timeout_clients(tcp_server_t* xserver) {
     int conn_timeout = (int) (xserver->config.conn_timeout_seconds * 1000); // in milliseconds

     /* ������õĳ�ʱʱ�䲻����0, ��ȡ�����ӳ�ʱ��� */
     if(conn_timeout <= 0)
         return;
         
     struct lh_entry *e, *tmp;

     /* ѭ��������ǰ�������� */
     lh_foreach_safe(_TCP_S_PRIVATE(xserver)->conns, e, tmp) {
         tcp_server_conn_t* conn = (tcp_server_conn_t*) e->k;

         /* ��鵱ǰʱ�����ϴ�ͨ��ʱ��Ĳ��  */
         if(uv_now(xserver->uvloop) - conn->last_comm_time > conn_timeout) {
             /* �Ѿ���ʱ, ��رո����� */
             if(xserver->config.log_out)
                 fprintf(xserver->config.log_out, "[tcp-server] %s close connection %p for its long time silence\n",
                         xserver->config.name, &conn->uvclient);
             _uv_disconnect_client((uv_stream_t*) &conn->uvclient);
         } else {
             /* ��������  */
             continue; 
         }
     }
 }
 
 int tcp_server_iter_conns(tcp_server_t* xserver, TCP_S_ON_ITER_CONN on_iter_conn, void* userdata) {
     /* ����û�δ��������ص�����, ��ֱ�ӷ��ص�ǰ�������� */
     if(on_iter_conn) {
         struct lh_entry *e, *tmp;
         /* ������ǰ���еĽڵ�, ��ÿ���ڵ�ִ���û�����Ļص����� */
         lh_foreach_safe(_TCP_S_PRIVATE(xserver)->conns, e, tmp) {
             on_iter_conn(xserver, (tcp_server_conn_t*) e->k, userdata);
         }
     }
     return _TCP_S_PRIVATE(xserver)->conns->count;
 }

