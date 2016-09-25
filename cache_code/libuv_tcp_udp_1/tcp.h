/* 
 * File:   udp.h
 * Author: sundq
 *
 * Created on 2016年1月23日, 下午2:49
 */

#ifndef TCP_H
#define	TCP_H

#ifdef	__cplusplus
extern "C"
{
#endif
#include <uv.h>
#include "tcp_server.h"

#define tcp_client_on_open_func_t tcp_server_on_open_func_t
#define tcp_client_on_close_func_t tcp_server_on_close_func_t
#define tcp_client_on_data_func_t tcp_server_on_data_func_t
#define tcp_client_on_error_func_t tcp_server_on_error_func_t
#define tcp_client_on_data_send_func_t tcp_server_on_data_send_func_t

tcp_client_t *tcp_client_create(const char *server_addr, int server_port,
                                tcp_client_on_open_func_t,
                                tcp_client_on_data_func_t,
                                tcp_client_on_close_func_t,
                                tcp_client_on_error_func_t,
                                tcp_client_on_data_send_func_t);
void tcp_client_run();
void tcp_client_close(tcp_client_t *tcp_client);
void tcp_client_send_data(tcp_client_t *tcp_client, char *data, size_t size);
void tcp_client_close(tcp_client_t *tcp_client);

#ifdef	__cplusplus
}
#endif

#endif	/* TCP_H */

