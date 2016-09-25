
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <time.h>
#include <assert.h>
#include "tcp_common.h"
#include "udp.h"



/* ������common.h�� */ 
void cmm__on_alloc_buf(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);

/* libuv ���յ����ݵĻص����� */
static void uvx__on_udp_recv(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned int flags) {
    uvx_udp_t* xudp = (uvx_udp_t*) handle->data;
    /* ������ĵĳ���Ϊ0, ��addr����NULL, ˵���յ��յ�UDP����(ֻ��UDPͷ, û��UDP��������)
       Ҳ�������ı��� 
       ���յ�UDP���ĺ�, �������ȫ�������, libuv�����������ȡ���ݻص�, ��ʱ
       nreadֵΪ0, addrΪNULL, ����ָʾ����UDP�����Ѿ���ȡ���
       */
    if(nread >= 0)
    {
        /* �����û�ע������ݴ���ص����� */
        if (xudp->config.on_recv)
        {
            xudp->config.on_recv(xudp, buf->base, nread, addr, flags);
        }    
        free(buf->base);
    }
    else
    {
        if(xudp->config.log_err)
            fprintf(xudp->config.log_err, "[uvx-udp] %s uvx__on_udp_recv error, nread = %zd, addr = %p ...\n", 
                            xudp->config.name, nread, addr);
    }
    
}

uvx_udp_config_t uvx_udp_default_config(uvx_udp_t* xudp) {
    uvx_udp_config_t config = { 0 };
    snprintf(config.name, sizeof(config.name), "udp-%p", xudp);
    config.log_out = stdout;
    config.log_err = stderr;
    return config;
}

int uvx_udp_start(uvx_udp_t* xudp, uv_loop_t* loop, const char* ip, int port, uvx_udp_config_t config) {
    assert(xudp && loop);
	xudp->uvloop = loop;
    memcpy(&xudp->config, &config, sizeof(uvx_udp_config_t));

	/* ��ʼ��udp uvʵ�� */
    uv_udp_init(loop, &xudp->uvudp);
    xudp->uvudp.data = xudp;

    /* �󶨱���ip�Ͷ˿� */
    if(ip) {
        int r;
        if(strchr(ip, ':')) {
            /* ipv6 ���� */
            struct sockaddr_in6 addr;
            uv_ip6_addr(ip, port, &addr);
            r = uv_udp_bind(&xudp->uvudp, (const struct sockaddr*) &addr, 0);
        } else {
            /* ipv4 ���� */
            struct sockaddr_in addr;
            uv_ip4_addr(ip, port, &addr);
            r = uv_udp_bind(&xudp->uvudp, (const struct sockaddr*) &addr, 0);
        }

        /* �󶨽����־���� */
        if(r >= 0 && config.log_out) {
            char timestr[32]; time_t t; time(&t);
            strftime(timestr, sizeof(timestr), "[%Y-%m-%d %X]", localtime(&t)); // C99 only: %F = %Y-%m-%d
            fprintf(config.log_out, "[uvx-udp] %s %s bind on %s:%d ...\n", timestr, xudp->config.name, ip, port);
        }
        if(r < 0) {
            if(config.log_err)
                fprintf(config.log_err, "\n!!! [uvx-udp] %s bind on %s:%d failed: %s\n", xudp->config.name, ip, port, uv_strerror(r));
            uv_close((uv_handle_t*) &xudp->uvudp, NULL);
            return 0;
        }
    } else {
        // if ip == NULL, default bind to local random port number (for UDP client)
        if(config.log_out)
            fprintf(config.log_out, "[uvx-udp] %s bind on local random port ...\n", xudp->config.name);
    }

    /* �������ݽ��մ��� */
    uv_udp_recv_start(&xudp->uvudp, cmm__on_alloc_buf, uvx__on_udp_recv);
    return 1;
}

static void uv_after_udp_send(uv_udp_send_t* req, int status) {
    /* ���ͷ��û���������ݻ����ڴ� */
    free(req->data);  

    /* Ȼ���ͷ��Լ������req */
    free(req); // see uvx_udp_send_to_addr()
}

int uvx_udp_send_to_addr(uvx_udp_t* xudp, const struct sockaddr* addr, const void* data, unsigned int datalen) {
    /* ����uv_udp_send_t, ���������͵������ڴ�ȥ��¼��data�ֶ���
       �Ա���������ڴ��ͷ�  */
    uv_udp_send_t* req = (uv_udp_send_t*) malloc(sizeof(uv_udp_send_t));
    uv_buf_t buf = uv_buf_init((char *)data, datalen);
    req->data = (void *)data;
    return (uv_udp_send(req, &xudp->uvudp, &buf, 1, addr, uv_after_udp_send) == 0 ? 1 : 0); 
}


int uvx_udp_send_to_ip(uvx_udp_t* xudp, const char* ip, int port, const void* data, unsigned int datalen) {
    assert(ip);
    if(strchr(ip, ':')) {
        struct sockaddr_in6 addr;
        uv_ip6_addr(ip, port, &addr);
        return uvx_udp_send_to_addr(xudp, (const struct sockaddr*) &addr, data, datalen);
    } else {
        struct sockaddr_in addr;
        uv_ip4_addr(ip, port, &addr);
        return uvx_udp_send_to_addr(xudp, (const struct sockaddr*) &addr, data, datalen);
    }
}


int uvx_udp_shutdown(uvx_udp_t* xudp) {
    /* ����ֹͣ���մ���, Ȼ��ر�udp uv��� */
    uv_udp_recv_stop(&xudp->uvudp);
    uv_close((uv_handle_t*) &xudp->uvudp, NULL);
    return 1;
}


