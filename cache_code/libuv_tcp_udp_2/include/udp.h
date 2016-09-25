#ifndef __UDP_H__
#define __UDP_H__
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
/* ��ͷ�ļ�����UDP ͨ�� ��ص����ݽṹ�Լ��ӿں���
   ��TCP��ͬ, UDP�����Ӳ���, ��˲���Ҫ����client��server
*/  

typedef struct uvx_udp_s uvx_udp_t;

/* 
���ܶ���: UDP �ڵ��յ����ݻص������� 
�Ƿ���붨��: ��ѡ����, �û�δ����ص�ʱ, ���ݽ�������
��������: 
uvx_udp_t* xudp  udp���, ������ο�uvx_udp_t���ݶ���
void* data : �յ���������ָ���׵�ַ, ���ֽ���������datalenָ��
ssize_t datalen: �յ������������ֽڳ���
const struct sockaddr* addr   ���յ������ݵ���Դ�˵�ַ, ����ip��ַ��˿�
unsigned int flags  ��ǰ���ݵ�ָʾ��Ϣ, ��ǰ�汾ֻ֧��UV_UDP_PARTIAL(2)��0����ȡֵ
                    ���flagsȡֵΪ0, ��˵����ǰ����UDP��������
                    ���flagsȡֵΪUV_UDP_PARTIAL(2), ˵����ǰϵͳUDP���ջ�����̫С,
                    ���յ��ı����Ѿ����ض�, �û��յ��ı����ǲ��ֱ���, ��Ҫ����UDP socket��������С
*/
typedef void (*UVX_UDP_ON_RECV) (uvx_udp_t* xudp, 
                                 void* data, 
                                 ssize_t datalen, 
                                 const struct sockaddr* addr, 
                                 unsigned int flags);

/* udp ʵ��������Ϣ */
typedef struct uvx_udp_config_s {
    char name[32];              /* udpʵ����, ����Ĭ��Ϊ udp-������� */
    UVX_UDP_ON_RECV on_recv;    /* udp���ݱ��Ľ��ջص�������, ��ѡ */
    FILE* log_out;
    FILE* log_err;
} uvx_udp_config_t;

/* udp ʵ�����ݽṹ */
struct uvx_udp_s {
    uv_loop_t* uvloop;          /* udp�����Ӧ��uvloop��� */ 
    uv_udp_t   uvudp;           /* udp�����Ӧ��uv_udp_t */ 
    uvx_udp_config_t config;    /* udp�����Ӧ��uvx_udp_config_t */ 
    void* data;                 /* �û���չ������, ���Զ�������, �ڻص������д��� */
};


/*
������������: ����UDPͨ�Žڵ��Ĭ��������Ϣ, �û����øú���������һ��Ĭ�����õ�
              uvx_udp_config_t, Ȼ������Լ���Ҫ�޸����в��ֲ���
��������:     uvx_udp_t* xudp  �û������uvx_udp_tָ��
����ֵ����:   ����һ�����Ĭ��ֵ��uvx_udp_config_t�ṹ
*/
uvx_udp_config_t uvx_udp_default_config(uvx_udp_t* xudp);


/*
������������: ����UDPͨ�Žڵ�
��������:     uvx_udp_t* xudp  �û������uvx_udp_tָ��
              uv_loop_t* loop  �û������uv_loop_tָ��, ͨ������libuv�ӿں������
              const char* ip   UDP ͨ�Žڵ�������ip��ַ, ���������ϵͳ�����е�ip��ַ,
                               ����0.0.0.0
              int port         UDP ͨ�Žڵ������Ķ˿�
              uvx_udp_config_t config  UDPͨ�Žڵ��������Ϣ
����ֵ����:   �ɹ����� 1
              ʧ�ܷ��� 0
*/
int uvx_udp_start(uvx_udp_t* xudp, uv_loop_t* loop, 
                  const char* ip, int port, 
                  uvx_udp_config_t config);


/*
������������: UDPͨ�Žڵ���ⷢ������
��������:     uvx_udp_t* xudp  �û������uvx_udp_tָ��
              const char* ip   ��������Ŀ��ip��ַ, ֧��ipv4��ipv6
              int port         ��������Ŀ�Ķ˿�
              uvx_udp_config_t config  UDPͨ�Žڵ��������Ϣ
              void* data   �û������͵������׵�ַ, �õ�ַ����Ϊ�û�ͨ��
             malloc����Ķ��ڴ�, ��������ʱ�ı����ڴ�; �û��ڵ���uvx_udp_send_to_ip
             �����ٶԴ˿��ڴ����κδ���, �ڴ��ͷ�������ڲ��������;
              unsigned int size      �û������͵������ֽ�����              
����ֵ����:   �ɹ����� 1
              ʧ�ܷ�Χ 0
*/
int uvx_udp_send_to_ip(uvx_udp_t* xudp, 
                       const char* ip, int port, 
                       const void* data, unsigned int datalen);
/*
������������: UDPͨ�Žڵ���ⷢ������
��������:     uvx_udp_t* xudp  �û������uvx_udp_tָ��
              const struct sockaddr* addr  ��������Ŀ��ͨ�ŵ�ַ
              uvx_udp_config_t config  UDPͨ�Žڵ��������Ϣ
              void* data   �û������͵������׵�ַ, �õ�ַ����Ϊ�û�ͨ��
             malloc����Ķ��ڴ�, ��������ʱ�ı����ڴ�; �û��ڵ���uvx_udp_send_to_ip
             �����ٶԴ˿��ڴ����κδ���, �ڴ��ͷ�������ڲ��������;
              unsigned int size      �û������͵������ֽ�����              
����ֵ����:   �ɹ����� 1
              ʧ�ܷ�Χ 0
*/                       
int uvx_udp_send_to_addr(uvx_udp_t* xudp, 
                         const struct sockaddr* addr, 
                         const void* data, unsigned int datalen);




/*
������������: �ر�UDPͨ�Žڵ�
��������:     uvx_udp_t* xudp  �û������uvx_udp_tָ��            
����ֵ����:   �ɹ����� 1
              ʧ�ܷ�Χ 0
*/
int uvx_udp_shutdown(uvx_udp_t* xudp);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
