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
/* ��ͷ�ļ�����tcp server ��ص����ݽṹ�Լ��ӿں��� */ 

typedef struct tcp_server_s tcp_server_t;
typedef struct tcp_server_conn_s tcp_server_conn_t;

/*tcp server �Ļص��������ӿڶ��� */

/* 
���ܶ���: TCP server �ɹ���������ʱ�ص������� 
�Ƿ���붨��: ��ѡ����, �û�δ����ص�ʱ���ӽ����ճɹ���������
��������: 
tcp_server_t* server  tcp server���, ������ο�tcp_server_t���ݶ���
tcp_server_conn_t* conn  tcp ���Ӿ��, ������ο�tcp_server_conn_t���ݶ���

*/
typedef void (*TCP_S_ON_CONN_OK)        (tcp_server_t* xserver, tcp_server_conn_t* conn);

/* 
���ܶ���: TCP server accept����ʧ�ܻص������� 
�Ƿ���붨��: ��ѡ����, �û�δ����ص�ʱ���ӽ���������ʧ�ܼ�������
��������: 
tcp_server_t* server  tcp server���, ������ο�tcp_server_t���ݶ���
tcp_server_conn_t* conn  tcp ���Ӿ��, ������ο�tcp_server_conn_t���ݶ���
*/
typedef void (*TCP_S_ON_CONN_FAIL)      (tcp_server_t* xserver, tcp_server_conn_t* conn);

/* 
���ܶ���: TCP server ���ӹر�ǰ�Ļص������� 
�Ƿ���붨��: ��ѡ����, �û�δ����ص�ʱ���ӽ��������Ӽ����رմ���
��������: 
tcp_server_t* server  tcp server���, ������ο�tcp_server_t���ݶ���
tcp_server_conn_t* conn  tcp ���Ӿ��, ������ο�tcp_server_conn_t���ݶ���
*/
typedef void (*TCP_S_ON_CONN_CLOSING)   (tcp_server_t* xserver, tcp_server_conn_t* conn);

/* 
���ܶ���: TCP server ���ӹر���ɺ�Ļص������� 
�Ƿ���붨��: ��ѡ����
��������: 
tcp_server_t* server  tcp server���, ������ο�tcp_server_t���ݶ���
tcp_server_conn_t* conn  tcp ���Ӿ��, ������ο�tcp_server_conn_t���ݶ���
*/
typedef void (*TCP_S_ON_CONN_CLOSE)     (tcp_server_t* xserver, tcp_server_conn_t* conn);


/* 
���ܶ���: TCP server  �յ�����ʱ�ص������� 
�Ƿ���붨��: ��ѡ����, ����û�δ����ûص�, �����յ������ݽ��ᱻ����;
              ����û������˸ûص�����, ����ͨ�����������������ȡ�յ�������
��������: 
tcp_server_t* server  tcp server���, ������ο�tcp_server_t���ݶ���
tcp_server_conn_t* conn  tcp ���Ӿ��, ������ο�tcp_server_conn_t���ݶ���
void* data : �յ���������ָ���׵�ַ, ���ֽ���������datalenָ��
ssize_t datalen: �յ������������ֽڳ���
*/
typedef void (*TCP_S_ON_RECV)           (tcp_server_t* xserver, tcp_server_conn_t* conn, void* data, ssize_t datalen);

/* 
���ܶ���: TCP server ��������ʱ�䵽��ʱ�Ļص�������
�ö�ʱ�����û�����tcp_server_start��ʼ����, ��������ʱ����tcp_server_config_t.heartbeat_interval_seconds
����; 
��������Ϊ�ϲ�Ӧ���ṩ��һ���򵥵Ķ�ʱ��, �û������ڸú�����ִ����Ҫ���ڽ��е��߼�����; 
����ڲ�ʹ�������������Ա�����鵱ǰserver�Ѿ������������������Ƿ�����г�ʱ��δͨ�ŵ�
����, ��������������ر�����;
�Ƿ���붨��: ��ѡ����
��������: 
tcp_server_t* server  tcp server���, ������ο�tcp_server_t���ݶ���
unsigned int index  �������ڻص�����ִ�д���, ���û�����tcp_server_start������ʼ����, 
                    ��ʼֵΪ0, ����һ���������ڻص�������ֵ����1;                    
*/
typedef void (*TCP_S_ON_HEARTBEAT)      (tcp_server_t* xserver, unsigned int index);

typedef struct tcp_server_config_s {
    char name[32];          /* TCP server���� */
    int conn_count;         /* server֧�ֽ����������������, Ĭ��Ϊ1024�� */
    int conn_backlog;       /* server �ײ�socket�ܻ������������������������
                               ���� uv_listen(), Ĭ��ֵ128 */
    int conn_extra_size;    /* tcp_server_conn_t.extra ���ӽṹ��չ�������Ĵ�С
                               Ĭ��ֵΪ0 */
    float conn_timeout_seconds; /* �ò�������ָ�����ӵ����ͨ��ʱ��, TCP server
                                  �ᶨ�ڼ�齨����ÿ������, ����������Ѿ�����conn_timeout_seconds
                                  δ��������ͨ��, ��TCP server�������رո�����
                                  Ĭ��ֵ180��, �����ֵΪ0, ��TCP server�����������Ƿ�ʱ 
                                  */
    float heartbeat_interval_seconds; /* TCP server ���ڶ�ʱ��ʱ��
                                        Ĭ��Ϊ60��*/
    // callbacks
    TCP_S_ON_CONN_OK        on_conn_ok;     /* ���Ӵ����ɹ��ص�, ��ѡ */
    TCP_S_ON_CONN_FAIL      on_conn_fail;   /* ���Ӵ���ʧ�ܻص�, ��ѡ */
    TCP_S_ON_CONN_CLOSING   on_conn_closing;/* ���ӹر�ǰ�ص�, ��ѡ */
    TCP_S_ON_CONN_CLOSE     on_conn_close;  /* ���ӹر���ɺ�ص�, ��ѡ */
    TCP_S_ON_HEARTBEAT      on_heartbeat;   /* ���������ص�����, ��ѡ */
    TCP_S_ON_RECV           on_recv;        /* SERVER�յ����ݻص�, ��ѡ */
    // logs
    FILE* log_out;
    FILE* log_err;
} tcp_server_config_t;

/* TCP serverʵ��������ݽṹ, ÿ��TCP server��Ӧһ��tcp_server_s */
struct tcp_server_s {
    uv_loop_t* uvloop;          /* ��server�����Ӧ��uvloop��� */  
    uv_tcp_t   uvserver;        /* ��server�����Ӧ��uvserver��� */  
    tcp_server_config_t config; /* ��server�����Ӧ��config */ 
    unsigned char privates[136]; /* ��server�����Ӧ���ڲ��������� */ 
    void* data;                 /* ��server�����Ӧ��Ӧ����չ������ */
};
typedef struct tcp_server_s tcp_server_t;

/* TCP server ����ʵ��������ݽṹ, ÿ��TCP ���Ӷ�Ӧһ��tcp_server_conn_s */
typedef struct tcp_server_conn_s {
    tcp_server_t* xserver;      /* ���Ӿ�� ��Ӧ��server��� */
    uv_tcp_t uvclient;          /* ���Ӿ�� ��Ӧ��client��� */
    uint64_t last_comm_time;    /* �������һ��ͨ�ŵ�ʱ��, ���ڼ�������Ƿ�ʱ */
    int refcount;               /* ���ӱ����ô���, �û��㲻Ҫֱ���޸ĸò���, ��Ӧ��
                                   ͨ������tcp_server_conn_ref�ӿڷ��� */
    uv_mutex_t refmutex;        /* refcount�Ĳ��������ź��� */ 
    void* extra;                /* ���Ӿ�� �е��û���չ������, ��С�� 
                                    tcp_server_config_t.conn_extra_size ����, 
                                    Ĭ��ΪNULL */               
} tcp_server_conn_t;

/*
������������: ����TCP server��Ĭ��������Ϣ, �û����øú���������һ��Ĭ�����õ�
              tcp_server_config_t, Ȼ������Լ���Ҫ�޸����в��ֲ���
��������:     tcp_server_t* xserver  �û������tcp_server_tָ��
����ֵ����:   ����һ�����Ĭ��ֵ��tcp_server_config_t�ṹ
*/
tcp_server_config_t tcp_server_default_config(tcp_server_t* xserver);



/*
������������: ����TCP server
��������:     tcp_server_t* xserver  �û������tcp_server_tָ��
              uv_loop_t* loop        �û������uv_loop_tָ��, ͨ������libuv�ӿں������
              const char* ip         TCP server������ip��ַ, ���������ϵͳ�����е�ip��ַ,
                                     ����0.0.0.0
              int port               TCP server�����Ķ˿�
              tcp_server_config_t config TCP server���е����úʹ���ص�
����ֵ����:   �ɹ����� 1
              ʧ�ܷ�Χ 0
*/
int tcp_server_start(tcp_server_t* xserver, 
                     uv_loop_t* loop, 
                     const char* ip, 
                     int port, 
                     tcp_server_config_t config);


/*
������������: �ر�TCP server
��������:     tcp_server_t* xserver  �û������tcp_server_tָ��
����ֵ����:   �ɹ����� 1
              ʧ�ܷ�Χ 0
*/
int tcp_server_shutdown(tcp_server_t* xserver);

/* 
���ܶ���: �û�����tcp_server_iter_conns��������TCP server ���ӵı����ص������� 
�Ƿ���붨��: �����tcp_server_iter_conns����˵������ʹ��
��������: 
tcp_server_t* server  tcp server���, ������ο�tcp_server_t���ݶ���
tcp_server_conn_t* conn  tcp ���Ӿ��, ������ο�tcp_server_conn_t���ݶ���
void* userdata  �û�������Զ���������, �������û����ж���
*/
typedef void (*TCP_S_ON_ITER_CONN)      (tcp_server_t* xserver, tcp_server_conn_t* conn, void* userdata);


/*
������������: ����TCP server��ǰ��������������
��������:     tcp_server_t* xserver  �û������tcp_server_tָ��
              TCP_S_ON_ITER_CONN on_iter_conn  �û���������ӱ����ص�����, �ò���
                                            ���ΪNULL, �򱾺��������б�������;
              void* userdata    �û���չ����ָ��
����ֵ����:   ���ص�ǰʵ�ʽ�������������
*/
int tcp_server_iter_conns(tcp_server_t* xserver, TCP_S_ON_ITER_CONN on_iter_conn, void* userdata);

/*
������������: TCP�������ô�������ӿ�, ���ڶ��ģ�������ͬһ������ʱ
              ������A/B/C/D�ĸ�ģ���������ͬһ������conn, ���ģ���ڳ��и�����ǰ
              ����tcp_server_conn_ref�����ü�����1, ��ģ���ͷŸ�����ʱ, ����tcp_server_conn_ref
              �����ü�����1;
��������:     tcp_server_conn_t* conn   ��Ҫ����������ָ��
              int ref                   ���ü����仯ֵ, ��ֵ����Ϊ1��-1
����ֵ����:   ��
            
*/
void tcp_server_conn_ref(tcp_server_conn_t* conn, int ref);

/*
������������: TCP server �����������ݽӿ�
��������:     tcp_server_conn_t* conn   ��Ҫ����������ָ��
              void* data   �û������͵������׵�ַ, �õ�ַ����Ϊ�û�ͨ��
             malloc����Ķ��ڴ�, ��������ʱ�ı����ڴ�; �û��ڵ���tcp_server_conn_send
             �����ٶԴ˿��ڴ����κδ���, �ڴ��ͷ��ɱ���ܸ������;
              unsigned int size      �û������͵������ֽ�����

����ֵ����:  �ɹ����� 1
             ʧ�ܷ�Χ 0
��ע: �ú���Ϊ�첽���ݷ��ͺ���, ���سɹ������������ݷ��ͳɹ�;  

*/
int tcp_server_conn_send(tcp_server_conn_t* conn, void* data, unsigned int size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* TCP_SERVER_H */
