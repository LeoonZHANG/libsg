/* 
 * File:   udp.h
 * Author: sundq
 *
 * Created on 2016年1月26日, 下午9:57
 */

#ifndef UDP_H
#define	UDP_H

#ifdef	__cplusplus
extern "C"
{
#endif
    
#include "udp_server.h"

udp_socket_t *udp_handle_client_create(udp_handle_on_data_func_t on_data, 
                                       udp_handle_on_send_func_t on_send, 
                                       udp_handle_on_error_func_t on_error);

#ifdef	__cplusplus
}
#endif

#endif	/* UDP_H */

