#include "udp.h"


/*
 功能： 创建udp客户端实例
 参数：
 *  on_data: 接收数据的回调函数
 *  on_error: 出错回调函数
 *  on_send: 异步发送数据完成后的回调函数
 返回结果：udp客户端实例
 */
udp_socket_t *udp_handle_client_create(udp_handle_on_data_func_t on_data, 
									   udp_handle_on_send_func_t on_send, 
									   udp_handle_on_error_func_t on_error)
{
	return udp_handle_server_create(NULL, 0, 1 , on_data, on_send, on_error);
}
