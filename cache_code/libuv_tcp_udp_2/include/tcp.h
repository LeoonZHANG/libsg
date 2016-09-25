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
��ͷ�ļ�����tcp client ��ص����ݽṹ�Լ��ӿں���
*/

typedef struct tcp_client_s tcp_client_t;

/*tcp client �Ļص��������ӿڶ��� */

/* 
���ܶ���: TCP client connect server�ɹ�ʱ�ص������� 
�Ƿ���붨��: ��ѡ����, �û�δ����ص�ʱ���ӽ����ճɹ���������
��������: 
tcp_client_t* xclient  tcp client���, ������ο�tcp_client_t���ݶ���
*/
typedef void (*TCP_C_ON_CONN_OK)        (tcp_client_t* xclient);    

/* 
���ܶ���: client connect serverʧ��ʱ�ص������� 
�Ƿ���붨��: ��ѡ����, �û�δ����ص�ʱ���ӽ���������ʧ�ܼ�������
��������: 
tcp_client_t* xclient  tcp client���, ������ο�tcp_client_t���ݶ���
*/
typedef void (*TCP_C_ON_CONN_FAIL)      (tcp_client_t* xclient);

/* 
���ܶ���: client ���ӹر�ǰ�Ļص������� 
�Ƿ���붨��: ��ѡ����, �û�δ����ص�ʱ���ӽ��������Ӽ����رմ���
��������: 
tcp_client_t* xclient  tcp client���, ������ο�tcp_client_t���ݶ���
*/
typedef void (*TCP_C_ON_CONN_CLOSING)   (tcp_client_t* xclient);

/* 
���ܶ���: client ���ӹر���ɺ�Ļص������� 
�Ƿ���붨��: ��ѡ����
��������: 
tcp_client_t* xclient  tcp client���, ������ο�tcp_client_t���ݶ���
*/
typedef void (*TCP_C_ON_CONN_CLOSE)     (tcp_client_t* xclient);

/* 
���ܶ���: client �յ�����ʱ�ص������� 
�Ƿ���붨��: ��ѡ����, ����û�δ����ûص�, �����յ������ݽ��ᱻ����;
              ����û������˸ûص�����, ����ͨ�����������������ȡ�յ�������
��������: 
tcp_client_t* xclient  tcp client���, ������ο�tcp_client_t���ݶ���
void* data : �յ���������ָ���׵�ַ, ���ֽ���������datalenָ��
ssize_t datalen: �յ������������ֽڳ���
*/
typedef void (*TCP_C_ON_RECV)           (tcp_client_t* xclient, void* data, ssize_t datalen);

/* 
���ܶ���: client ��������ʱ�䵽��ʱ�Ļص�������
�ö�ʱ���ӳ��Դ������ӿ�ʼ����, ��������ʱ����tcp_client_config_t.heartbeat_interval_seconds
����; �������ں���Ϊ�ϲ�Ӧ���ṩ��һ���򵥵Ķ�ʱ��, �û������ڸú�����ִ����Ҫ����
���е��߼�����; 
ͬʱ, tcp�ͻ��������ö�ʱ��������ʧ�ܻ������쳣�Ͽ�ʱ�Զ���������
�Ƿ���붨��: ��ѡ����, ����û�δ����ûص�, ��;
              ����û������˸ûص�����, ����ͨ�����������������ȡ�յ�������
��������: 
tcp_client_t* xclient  tcp client���, ������ο�tcp_client_t���ݶ���
unsigned int index  �������ڻص�����ִ�д���, ��client����connect������ʼ����, 
                    ��ʼֵΪ0, ����һ���������ڻص�������ֵ����1;                    
*/
typedef void (*TCP_C_ON_HEARTBEAT)      (tcp_client_t* xclient, unsigned int index);

/*
tcp client Ӧ�ÿ��ƽṹ, ���ṹΪӦ�ò���Ϊ��Ҫ�����ݽӿ�
*/
typedef struct tcp_client_config_s {
    char name[32];       /* ��ǰclient������, �û����Զ���, Ĭ��ֵΪ
                            tclient-�������, ��Ҫ������־��ʾ��ͬ��client */
    int retry_times;      /* tcp client����ʧ�ܺ����Դ���, Ĭ��Ϊ5��*/
    float heartbeat_interval_seconds;   /* ��������ʱ��, ��ͨ��С����֧�ֺ��� */
    // callbacks
    TCP_C_ON_CONN_OK       on_conn_ok;  /* ���Ӵ����ɹ��ص�, ��ѡ */
    TCP_C_ON_CONN_FAIL     on_conn_fail;/* ���Ӵ���ʧ�ܻص�, ��ѡ */
    TCP_C_ON_CONN_CLOSING  on_conn_closing; /* ���ӹر�ǰ�ص�, ��ѡ */
    TCP_C_ON_CONN_CLOSE    on_conn_close;   /* ���ӹر���ɺ�ص�, ��ѡ */
    TCP_C_ON_RECV          on_recv;         /* �յ�SERVER���ݻص�, ��ѡ */
    TCP_C_ON_HEARTBEAT     on_heartbeat;    /* ���������ص�����, ��ѡ */
    // logs
    FILE* log_out;                          
    FILE* log_err;
} tcp_client_config_t;

/* TCP client ������ݽṹ, ÿ��TCP client��Ӧһ��tcp_client_s */
struct tcp_client_s {
    uv_loop_t* uvloop;            /* ��client�����Ӧ��uvloop��� */       
    uv_tcp_t   uvclient;          /* ��client�����Ӧ��uv_tcp_t client��� */ 
    uv_tcp_t*  uvserver;          /* ��client�����Ӧ��uv_tcp_t server��� */
    tcp_client_config_t config;   /* ��client�����Ӧ��client������Ϣ�ṹ */
    unsigned char privates[296];  /* ��client�����Ӧ��client�ڲ�������Ϣ */
    void* data;                   /* ��client�����Ӧ���û��Զ���������, �û���
                                     ���������ڴ洦�� */
};
typedef struct tcp_client_s tcp_client_t;

/*
������������: ����TCP client��Ĭ��������Ϣ, �û����øú���������һ��Ĭ�����õ�
              tcp_client_config_t, Ȼ������Լ���Ҫ�޸����в��ֲ���
��������:     tcp_client_t* xclient  �û������tcp_client_tָ��
����ֵ����:   ����һ�����Ĭ��ֵ��tcp_client_config_t�ṹ
*/
tcp_client_config_t tcp_client_default_config(tcp_client_t* xclient);

/*
������������: TCP Client��������TCP server
��������:     tcp_client_t* xclient  �û������tcp_client_tָ��
              uv_loop_t* loop  �û������uv_loop_tָ��, ͨ������libuv�ӿں������
              const char* ip   server��ip��ַ�ַ���, ֧��ipv4��ipv6���ָ�ʽ
                               ipv4���� a.b.c.d
                               ipv6���� 2000:0000:0000:0000:0001:2345:6789:abcd
                                        ��2000::1:2345:6789:abcd
              int port         server�Ķ˿���Ϣ
              tcp_client_config_t config  client��������Ϣ, һ��ͨ��tcp_client_default_config
                                          ����, �����û��Զ����޸ĺ�ȷ��
����ֵ����:   1 ��������ɹ�
              0 ��������ʧ��, ʧ��ԭ��ο���־, ���м�¼��libuvʧ�ܵľ���ԭ��
��ע: �ú���Ϊ�첽���Ӻ���, ���سɹ���������TCP���ӽ����ɹ�, ֻ��˵��TCP���ӳ���
      �����ɹ�, TCP���ӵĳɹ���ʧ�ܽ����Ϣ��Ҫ�ȴ�tcp_client_config_t��ע���
      ���ӳɹ�/ʧ�ܻص�������ȡ
*/
int tcp_client_connect(tcp_client_t* xclient, 
                       uv_loop_t* loop, 
                       const char* ip, 
                       int port, 
                       tcp_client_config_t config);


/*
������������: TCP Client �������ݺ���
��������:     tcp_client_t* xclient  �û������tcp_client_tָ��
              void* data   �û������͵������׵�ַ, �õ�ַ����Ϊ�û�ͨ��
              malloc����Ķ��ڴ�, ��������ʱ�ı����ڴ�; �û��ڵ���tcp_client_send
              �����ٶԴ˿��ڴ����κδ���, �ڴ��ͷ��ɱ���ܸ������;
              unsigned int size      �û������͵������ֽ�����
����ֵ����:   1 ��������ɹ�
              0 ��������ʧ��, ʧ��ԭ��ο���־, ���м�¼��ʧ�ܵľ���ԭ��
��ע: �ú���Ϊ�첽���ݷ��ͺ���, ���سɹ������������ݷ��ͳɹ�;
*/
int tcp_client_send(tcp_client_t* xclient, void* data, unsigned int size);


/*
������������: TCP Client ��ʱ�����Ͽ����Ӻ���
��������:     tcp_client_t* xclient  �û������tcp_client_tָ��
����ֵ����:   1 ��������ɹ�
              0 ��������ʧ��, ʧ��ԭ��ο���־, ���м�¼��ʧ�ܵľ���ԭ��
��ע: ����û�ע���������ص���������������������, �ú�����Ͽ���server������;
      ��������������ʱ�䵽���Ժ�, �Զ����½�������; 
      ����û�ϣ�����׶Ͽ�����, ��ʹ��tcp_client_shutdown����
*/
int tcp_client_disconnect(tcp_client_t* xclient);


/*
������������: TCP Client �����Ͽ����Ӻ���
��������:     tcp_client_t* xclient  �û������tcp_client_tָ��
����ֵ����:   1 ��������ɹ�
              0 ��������ʧ��, ʧ��ԭ��ο���־, ���м�¼��ʧ�ܵľ���ԭ��
*/
int tcp_client_shutdown(tcp_client_t* xclient);




#ifdef __cplusplus
} // extern "C"
#endif

#endif /* TCP_SERVER_H */

