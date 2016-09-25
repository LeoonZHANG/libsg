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
TCP client��ַ����
*/
typedef union tcp_sockaddr_4_6_s{
    struct sockaddr_in  in4;            /* ipv4��ַ */
    struct sockaddr_in6 in6;            /* ipv6��ַ */
} tcp_sockaddr_4_6_t;

/*
TCP client ���˽����Ϣ���ݽṹ, ���治���ⲿ��¶����Ϣ
*/
typedef struct tcp_client_private_s {
    uv_connect_t conn;              /* ��server������������Ϣ*/ 
    tcp_sockaddr_4_6_t server_addr; /* server��ip�Ͷ˿���Ϣ */
    uv_timer_t heartbeat_timer;     /* ������ʱ����� */
    unsigned int heartbeat_index;   /* �������ڳ�ʱ������, ��¼���ڸ��� */
    int connection_closed;          /* ��¼client��server�������Ƿ�ر� */
    int retried_times;              /* �Ѿ��������ԵĴ��� */
} tcp_client_private_t;

/* ��ȡclient private�ṹ��, ���ٻ�ȡtcp_client_private_t��ַ */
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
        �����ص�����, ��������Ѿ�����, ������û�ע��Ļص�
                      �������δ����, �Զ���������
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
            /* ���xclient->config.retry_times ����Ϊ0, �����޴����� */
            if (xclient->config.retry_times <= 0)
            {
                tcp__client_reconnect(xclient);
            }
            else
            {
                /* �����������С����������������, ������������� */
                if (TCP__C_PRIVATE(xclient)->retried_times < xclient->config.retry_times)
                {
                    tcp__client_reconnect(xclient);
                }
                else
                {
                    fprintf(xclient->config.log_out, 
                            "[tcp-client] %s has retried %d times, stop trying\n", 
                            xclient->config.name, TCP__C_PRIVATE(xclient)->retried_times);
                    //ֹͣlibuv��ʱ��
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

    /* ������ǰ, ��˽�������ݳ�ʼ�� */
    TCP__C_PRIVATE(xclient)->connection_closed = 0;
    TCP__C_PRIVATE(xclient)->retried_times     = 0;
    memcpy(&xclient->config, &config, sizeof(tcp_client_config_t));
    if(strchr(ip, ':'))
        uv_ip6_addr(ip, port, (struct sockaddr_in6*) &TCP__C_PRIVATE(xclient)->server_addr);
    else
        uv_ip4_addr(ip, port, (struct sockaddr_in*) &TCP__C_PRIVATE(xclient)->server_addr);

    //�������ӳ���, �״����Ӻ���������tcp__client_reconnect����
    int ret = tcp__client_reconnect(xclient);
    if (ret == 1)
    {
        /*
            ������Ӳ�������ɹ�, �����������ڶ�ʱ��
        */
        int timeout = (int)(config.heartbeat_interval_seconds * 1000); // in milliseconds
        TCP__C_PRIVATE(xclient)->heartbeat_index = 0;
        TCP__C_PRIVATE(xclient)->heartbeat_timer.data = xclient;  

        //��ʼ��libuv��ʱ��������
    	uv_timer_init(loop, &TCP__C_PRIVATE(xclient)->heartbeat_timer);
    	uv_timer_start(&TCP__C_PRIVATE(xclient)->heartbeat_timer, tcp__on_heartbeat_timer, timeout, timeout);
    }
	return ret;
}


int tcp_client_send(tcp_client_t* xclient, void* data, unsigned int size) {
    /* �ж������Ƿ����ɹ�, �����ɹ��������� */
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
    /* ��ʼ��uvclient */
	uv_tcp_init(xclient->uvloop, &xclient->uvclient);

	/* ��������server, ���������ӻص����� */
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

    /* �����û������ӹرպ�ص�����, ��������Դ */
    if(xclient->config.on_conn_close)
        xclient->config.on_conn_close(xclient);
    xclient->uvserver = NULL;
    TCP__C_PRIVATE(xclient)->connection_closed = 1;
}

static void _tcp_client_close(tcp_client_t* xclient) {
    if(xclient->config.log_out)
        fprintf(xclient->config.log_out, "[tcp-client] %s on close\n", xclient->config.name);
    /* client �����������ӹرղ���, �����û����ӹر�ǰ�ص� */
    if(xclient->config.on_conn_closing)
        xclient->config.on_conn_closing(xclient);

    /* �رշ����uvclient handle��Դ */    
    uv_close((uv_handle_t*) &xclient->uvclient, tcp__after_close_client);
}


static void tcp__on_client_read(uv_stream_t* uvserver, ssize_t nread, const uv_buf_t* buf) {
    tcp_client_t* xclient = (tcp_client_t*) uvserver->data;
    assert(xclient);

    /* client��ǰ���������ݿɶ�, ���жϿɶ����ݴ�С */
	if(nread > 0) {
	    /* ����0, ��ֱ�ӵ����û�ע������ݶ�ȡ�ص� */
        assert(xclient->uvserver == (uv_tcp_t*)uvserver);
        if(xclient->config.on_recv)
            xclient->config.on_recv(xclient, buf->base, nread);
        /* ��nread>0ʱ, �������Ѿ��ṩ���û�, �˴���Ҫ�ͷŵ�bufָ�� */
        free(buf->base);    
	} else if(nread < 0) {
	    /* С��0, ˵����ǰ�����Ѿ��쳣, ֱ��ֹͣ��ȡ�ص�, ���ر�����
	       �ر�ע��: �� nread < 0ʱ, buf�������л�������, ����Ҫ�����ڴ��ͷ� */

	    /* ֹͣlibuv�Ķ��¼����� */   
		uv_read_stop(uvserver);
		if(xclient->config.log_err)
            fprintf(xclient->config.log_err, "\n!!! [tcp-client] %s on recv error: %s\n", xclient->config.name, uv_strerror(nread));
		_tcp_client_close(xclient); // will try reconnect on next tcp__on_heartbeat_timer()
	}   
}

// �ú�����ʵ�ֶ����� common.c ��
void cmm__on_alloc_buf(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);

static void _uv_on_connect(uv_connect_t* conn, int status) {
    tcp_client_t* xclient = (tcp_client_t*) conn->data;
    xclient->uvclient.data = xclient;

    /* ���ӽ���ص�������, ��status Ϊ 0ʱ, ���ӽ����ɹ�, �������ӽ���ʧ�� */
	if(status == 0) {
		if(xclient->config.log_out)
            fprintf(xclient->config.log_out, "[tcp-client] %s connect to server ok\n", xclient->config.name);
		assert(conn->handle == (uv_stream_t*) &xclient->uvclient);
		xclient->uvserver = (uv_tcp_t*) conn->handle;

        /* ���ӳɹ�, ������ʧ�������������� */
        TCP__C_PRIVATE(xclient)->retried_times = 0;
        
		/* �����û�ע������ӳɹ�����ص����� */
        if(xclient->config.on_conn_ok)
            xclient->config.on_conn_ok(xclient);

        /* ����libuv���¼�����, ��ע����ص� */    
		uv_read_start(conn->handle, cmm__on_alloc_buf, tcp__on_client_read);
	} else {
	    /* ���ӽ���ʧ�� */
		xclient->uvserver = NULL;
		if(xclient->config.log_err)
            fprintf(xclient->config.log_err, "\n!!! [tcp-client] %s connect to server failed: %s\n", xclient->config.name, uv_strerror(status));

        /* �����û�ע�������ʧ�ܴ���ص����� */
        if(xclient->config.on_conn_fail)
            xclient->config.on_conn_fail(xclient);

        /* todo: ȷ�ϴ˴���Ӧ�õ���_tcp_client_close, ��Ӧ��ֱ�ӹر�handle���� */
		_tcp_client_close(xclient); // will try reconnect on next on_heartbeat
	}
}

int tcp_client_disconnect(tcp_client_t* xclient) {
	_tcp_client_close(xclient);
	return 1;
}

int tcp_client_shutdown(tcp_client_t* xclient) {
    /* shutdown��������ֹͣ������ʱ�� */
	uv_timer_stop(&TCP__C_PRIVATE(xclient)->heartbeat_timer);
	_tcp_client_close(xclient);
	return 1;
}

