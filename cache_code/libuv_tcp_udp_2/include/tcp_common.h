
/*
 * tcp_common.h
 * High concurrency tcp server library based on libuv.
 */

#ifndef __TCP_COMMON_H__
#define __TCP_COMMON_H__
#ifdef __cplusplus
extern "C"	{
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <uv.h>

//-----------------------------------------------
// ���ļ����������ӿ�

/*
������������: ��һ��struct sockaddr�ṹ�ĵ�ַת��Ϊ�ɶ����ַ���ip��ַ�Ͷ˿ں�
��������:     const struct sockaddr* addr  ����ĵ�ַ
              char* ipbuf  �����ַת�����ip��ַ�ַ���,C���, ��0��β
                           ipbuf���ڴ���Ҫ�����߽��з���
              int buflen   ipbuf���ڴ泤��, ��Ҫ�����߽��к�������
                           ipv4��ַ���鲻С��16, ipv6��ַ���鲻С��40
              int* port    �����ַת����Ķ˿ں�, �õ��Ľ��Ϊ������
����ֵ����:   �����ɹ�ʱ���ص�ֵ����ipbuf
              ����ʧ��ʱ����NULL
*/
const char* cmm_get_ip_port(const struct sockaddr* addr, 
                            char* ipbuf, 
                            int buflen, 
                            int* port);


/*
������������: ��һ��struct sockaddr�ṹ�ĵ�ַת��Ϊ�����Ƶ�ip��ַ�Ͷ˿ں�
��������:     const struct sockaddr* addr  ����ĵ�ַ
              char* ipbuf  �����ַת�����ip��ַ�ַ���, ipbuf���ڴ���Ҫ�����߽��з���                           
                           ipv4��ַ�����СΪ4, ipv6��ַ�����СΪ16
              int* port    �����ַת����Ķ˿ں�, �õ��Ľ��Ϊ������
����ֵ����:   �����ɹ�ʱ > 0
              ����ʧ��ʱ����0
*/
int cmm_get_raw_ip_port(const struct sockaddr* addr, unsigned char* ipbuf, int* port);


/*
������������: ��һ��uv_tcp_t�ṹ�ľ��ת��Ϊ�ɶ����ַ�����ip��ַ�Ͷ˿ں�
��������:     uv_tcp_t* uvclient  �����uv_tcp_t���ָ��
              char* ipbuf  �����ַת�����ip��ַ�ַ���,C���, ��0��β
                           ipbuf���ڴ���Ҫ�����߽��з���
              int buflen   ipbuf���ڴ泤��, ��Ҫ�����߽��к�������
                           ipv4��ַ���鲻С��16, ipv6��ַ���鲻С��40
              int* port    �����ַת����Ķ˿ں�, �õ��Ľ��Ϊ������
����ֵ����:   �����ɹ�ʱ���ص�ֵ����ipbuf
              ����ʧ��ʱ����NULL

*/
const char* cmm_get_tcp_ip_port(uv_tcp_t* uvclient, char* ipbuf, int buflen, int* port);

/*
������������: ��һ���Ѿ��򿪵�uv_stream_t��д������
��������:     uv_stream_t* stream  �Ѿ������򿪵�uv_stream_t
              void* data   �û����������ݵ��ڴ��׵�ַ, data���ڴ���Ҫ�����߽��з���
                           data���ڴ��ͷŽ��ڱ����ú����ڲ����, ��˵������ڵ���
                           cmm_send_to_stream������, �����ٲ���void* dataָ����ڴ�                           
              unsigned int size   ���������ݵ��ֽڳ���

����ֵ����:   �����ɹ�ʱ���� 1
              ����ʧ��ʱ���� 0
*/
int cmm_send_to_stream(uv_stream_t* stream, void* data, unsigned int size);



#if defined(_WIN32) && !defined(__GNUC__)
#include <stdarg.h>
/* Windows�ϵ�snprintfģ�⺯�� */ 
int snprintf(char* buf, size_t len, const char* fmt, ...);
#endif


#ifdef __cplusplus
} // extern "C"
#endif

#endif

