#include "tcp.h"

static uv_loop_t *client_loop;

/*
 功能： 工具函数，用于接收数据时分配存储数据的内存的回调函数
 参数：
 *  handle: libuv的句柄，一般情况不适用
 *  suggested_size：建议分配的内存大小
 *  buff：返回结果
 返回结果：无
 */
static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buff)
{
	tcp_client_t *tcp_client = (tcp_client_t *) handle->data;
	buff->base = (char *) calloc(1, suggested_size);
	buff->len = suggested_size;
}

static void on_read(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf)
{
	tcp_client_t *tcp_client = (tcp_client_t *) handle->data;
	if (nread > 0)
	{
		if (tcp_client->on_data)
		{
			tcp_client->on_data(tcp_client, buf->base, nread);
		}
	}
	else if (nread == 0)
	{
		//nothing recv
	}
	else
	{
		fprintf(stderr, "read data failed\n");
		uv_read_stop(handle);
		uv_close((uv_handle_t*) handle, NULL);
		if (tcp_client->on_conn_close)
		{
			tcp_client->on_conn_close(tcp_client);
		}
		free(tcp_client->connect_req);
		free(tcp_client->client);
		free(tcp_client);
	}
	if (buf->base != NULL)
	{
		free(buf->base);
	}
}

static void on_connect(uv_connect_t *req, int status)
{
	tcp_client_t *tcp_client = (tcp_client_t *) req->data;
	if (status == -1)
	{
		fprintf(stderr, "error on_write_end");
		if (tcp_client->on_conn_error)
		{
			tcp_client->on_conn_error("error connect to remote", tcp_client);
		}
	}
	else
	{
		if (tcp_client->on_conn_open)
		{
			tcp_client->on_conn_open(tcp_client);
		}
	}
	req->handle->data = tcp_client;
	uv_read_start(req->handle, alloc_buffer, on_read);

}

static void on_close_connection(uv_handle_t* handle)
{
	tcp_client_t *tcp_client = (tcp_client_t *) handle->data;
	tcp_client->is_closed = 1;
	if (tcp_client->on_conn_close)
	{
		tcp_client->on_conn_close(tcp_client);
	}

	uv_loop_close(tcp_client->uvloop);
	free(tcp_client->connect_req);
	free(tcp_client->client);
	free(tcp_client);
}

static void on_client_write(uv_write_t *req, int status)
{
	if (status < 0)
	{
		fprintf(stderr, "Write error!\n");
	}

	tcp_send_data_cb_t *p_data = (tcp_send_data_cb_t *) req->data;
	tcp_client_t *tcp_client = p_data->tcp_client;
	if (tcp_client->on_send) //客户端回调函数调用
	{
		tcp_client->on_send(tcp_client, status, p_data->send_data.base, p_data->send_data.len);
	}	
    free(req);	
	free(p_data->send_data.base);
}

/*
 功能： 创建tcp客户端实例
 参数：
 *  server_addr: 服务地址
 *  server_port：服务端口
 *  on_open: 链接建立成功的回调函数
 *  on_data: 接收数据的回调函数
 *  on_close: 链接关闭的回调函数
 *  on_error: 出错回调函数
 *  on_send: 异步发送数据完成后的回调函数
 返回结果：tcp客户端实例
 */


tcp_client_t *tcp_client_create(const char *server_addr, int server_port,
								tcp_client_on_open_func_t on_open,
								tcp_client_on_data_func_t on_data,
								tcp_client_on_close_func_t on_close,
								tcp_client_on_error_func_t on_error,
								tcp_client_on_data_send_func_t on_send)
{
	if (client_loop == NULL)
	{
		client_loop = malloc(sizeof (uv_loop_t));
		uv_loop_init(client_loop);
	}
	
	tcp_client_t *tcp_client = (tcp_client_t *) calloc(1, sizeof (tcp_client_t));
	tcp_client->uvloop = client_loop;
	tcp_client->client = (uv_tcp_t *) calloc(1, sizeof (uv_tcp_t));
	tcp_client->connect_req = (uv_connect_t *) calloc(1, sizeof (uv_connect_t));
	tcp_client->on_conn_open = on_open;
	tcp_client->on_data = on_data;
	tcp_client->on_conn_close = on_close;
	tcp_client->on_conn_error = on_error;
	tcp_client->on_send = on_send;
	uv_tcp_init(tcp_client->uvloop, tcp_client->client);
	struct sockaddr send_addr = {0};
	if (strchr(server_addr, ':'))
	{
		uv_ip6_addr(server_addr, server_port, (struct sockaddr_in6 *) &send_addr);
	}
	else
	{
		uv_ip4_addr(server_addr, server_port, (struct sockaddr_in *) &send_addr);
	}
	tcp_client->connect_req->data = tcp_client;
	tcp_client->async_handle.data = tcp_client;
	uv_tcp_connect(tcp_client->connect_req, tcp_client->client, (const struct sockaddr *) &send_addr, on_connect);
	return tcp_client;
}

/*
 功能： 运行tcp客户端实例
 参数：
 *  tcp_client: tcp客户端实例
 返回结果：无
 */
void tcp_client_run()
{
	uv_run(client_loop, 0);
}

/*
 功能： 客户端发送数据到服务端
 参数：
 *  tcp_client: tcp客户端实例
 *  data: 数据指针
 *  size：数据大小
 返回结果：无
 说明：此函数调用会触发on_send回调函数 
 */
void tcp_client_send_data(tcp_client_t *tcp_client, char *data, size_t size)
{
	char *send_data = malloc(size);
	memcpy(send_data, data, size);
	tcp_server_send_data(tcp_client, data, size);
}

/*
 功能： 关闭tcp连接并失败tcp客户端实例
 参数：
 *  tcp_client: tcp客户端实例
 返回结果：无
 */
void tcp_client_close(tcp_client_t *tcp_client)
{
	if (tcp_client->is_closed)
	{
		if (tcp_client->on_conn_close)
		{
			tcp_client->on_conn_close(tcp_client);
		}
	}
	else
	{
		tcp_client->is_closed = 1;
		uv_close((uv_handle_t*) tcp_client->client, on_close_connection);
	}
}