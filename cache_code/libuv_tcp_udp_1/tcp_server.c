#include <stdio.h>
#include <stdlib.h>
#include "tcp.h"
#include "tcp_server.h"


static void perfomance_print(uv_timer_t *handle)
{
	tcp_server_t *tcp_server = (tcp_server_t *) handle->data;
    uv_rusage_t usage = {0};
    uv_getrusage(&usage);
	uint64_t mem_total = usage.ru_ixrss + usage.ru_isrss + usage.ru_idrss + usage.ru_maxrss;
	double time_elapse = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / (double)(1000 * 1000) + usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / (double)(1000 * 1000);  
	double throughout = 0.0;
	if (time_elapse > 0.0001 && tcp_server->bytes_recv > 0)
	{
		throughout = (tcp_server->bytes_recv * 8) / (double) (1024 * 1024) / time_elapse; //Mbps
		fprintf(stderr, "tcp server:concurrence=%d, time_elaspse=%f, memory=%0.2fkb, throughout=%.2fMbps \n", tcp_server->conn_count,time_elapse, mem_total/1024.0, throughout);
	}
}

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buff)
{
	buff->base = (char*) calloc(1, suggested_size);
	buff->len = suggested_size;
}

static void on_close_connection(uv_handle_t* handle)
{
	tcp_client_t *tcp_client = (tcp_client_t *) handle->data;
	tcp_server_t *tcp_server = (tcp_server_t *) tcp_client->server;
    tcp_client->is_closed = 1;
	if (tcp_server->on_conn_close)
	{
		tcp_server->on_conn_close(tcp_client);
	}

	if (tcp_client->on_conn_close)
	{
		tcp_server->on_conn_close(tcp_client);
	}
	free(tcp_client);
	free(handle);
}

static void on_client_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
	tcp_client_t *tcp_client = (tcp_client_t *) client->data;
	tcp_server_t *tcp_server = (tcp_server_t *) tcp_client->server;
	if (nread > 0)
	{
		if (tcp_server->on_data)
		{
			tcp_server->on_data(tcp_client, buf->base, nread);
		}
		tcp_server->bytes_recv += nread;
	}
	else if (nread == 0)
	{
		//nothing recv
	}
	else
	{
		if (tcp_client->is_closed == 0)
		{
			uv_read_stop(client);
			uv_close((uv_handle_t*) client, on_close_connection);
			tcp_server->conn_count--;
			if (tcp_server->on_conn_error)
			{
				tcp_server->on_conn_error("error on_client_read", tcp_client);
			}
			tcp_client->is_closed = 1;
		}
	}
	if (buf->base != NULL)
	{
		free(buf->base);
	}
}

static void on_client_write(uv_write_t *req, int status)
{
	if (status < 0)
	{
		fprintf(stderr, "Write error!\n");
	}

	tcp_send_data_cb_t *p_data = (tcp_send_data_cb_t *) req->data;
	if (p_data->tcp_client->server && p_data->tcp_client->server->on_send) //服务端回调函数调用
	{
		p_data->tcp_client->server->on_send(p_data->tcp_client, status, p_data->send_data.base, p_data->send_data.len);
		p_data->tcp_client->server->bytes_send += p_data->send_data.len;
	}
	if (p_data->tcp_client->on_send) //客户端回调函数调用
	{
		p_data->tcp_client->on_send(p_data->tcp_client, status, p_data->send_data.base, p_data->send_data.len);
	}
	free(req);
}

static void on_new_connection(uv_stream_t *server, int status)
{
	tcp_server_t *tcp_server = (tcp_server_t *) server->data;
	if (status == -1)
	{
		if (tcp_server->on_conn_error)
		{
			tcp_server->on_conn_error("error on_new_connection", NULL);
		}
		return;
	}

	uv_tcp_t *client = (uv_tcp_t*) calloc(1, sizeof (uv_tcp_t));
	uv_tcp_init(tcp_server->uvloop, client);

	int result = uv_accept(server, (uv_stream_t*) client);
	if (result == 0)
	{
		if (tcp_server->conn_count >= tcp_server->max_connections)
		{
			fprintf(stderr, "too many connections, the max connection is:%d\n", tcp_server->max_connections);
			uv_close((uv_handle_t*) client, on_close_connection);
			return;
		}

		tcp_client_t *tcp_client = calloc(1, sizeof (tcp_client_t));
		tcp_client->server = tcp_server;
		tcp_client->client = client;
		client->data = tcp_client;
		tcp_server->conn_count++; //服务连接数
		if (tcp_server->on_conn_open)
		{
			tcp_server->on_conn_open(tcp_client);
		}
		uv_read_start((uv_stream_t*) client, alloc_buffer, on_client_read);
	}
	else
	{
		if (tcp_server->on_conn_error)
		{
			tcp_server->on_conn_error("accept connction failed.", NULL);
			perror("accept connection failed");
		}
	}
}

/*
 功能： 创建tcp服务端实例
 参数：
 *  server_addr: 服务监听的地址
 *  server_port：服务监听的端口
 *  mode: 0 生产模式 1 开发模式(在此模式下回打印吞吐量的并发量)
 *  on_open: 链接建立成功的回调函数
 *  on_data: 接收数据的回调函数
 *  on_close: 链接关闭的回调函数
 *  on_error: 出错回调函数
 *  on_send: 异步发送数据完成后的回调函数
 返回结果：tcp服务端实例
 */

tcp_server_t *tcp_server_create(const char *server_addr,
								int server_port,
								int backlog,
								int mode,
								tcp_server_on_open_func_t on_open,
								tcp_server_on_close_func_t on_close,
								tcp_server_on_error_func_t on_error,
								tcp_server_on_data_func_t on_data,
								tcp_server_on_data_send_func_t on_send)
{
	int r = 0;
	tcp_server_t *tcp_server = (tcp_server_t *) calloc(1, sizeof (tcp_server_t));
	tcp_server->uvloop = malloc(sizeof (uv_loop_t));
	tcp_server->on_conn_open = on_open;
	tcp_server->on_conn_close = on_close;
	tcp_server->on_conn_error = on_error;
	tcp_server->on_data = on_data;
	tcp_server->on_send = on_send;
	tcp_server->max_connections = 10;

	uv_tcp_t *server = &(tcp_server->server);
	uv_loop_init(tcp_server->uvloop);
	uv_tcp_init(tcp_server->uvloop, server);
	struct sockaddr bind_addr = {0};
	if (strchr(server_addr, ':'))
	{
		uv_ip6_addr(server_addr, server_port, (struct sockaddr_in6 *) &bind_addr);
	}
	else
	{
		uv_ip4_addr(server_addr, server_port, (struct sockaddr_in *) &bind_addr);
	}

	uv_tcp_bind(server, (const struct sockaddr*) &bind_addr, 0);
	server->data = tcp_server;
	r = uv_listen((uv_stream_t*) server, backlog, on_new_connection);
	if (r)
	{
		fprintf(stderr, "listen tcp server failed:%d\n", r);
		perror("listen failed");
		return NULL;
	}

	if (mode == 1)
	{
		tcp_server->timer.data = tcp_server;
		r = uv_timer_init(tcp_server->uvloop, &tcp_server->timer);
		if (r)
		{
			fprintf(stderr, "create timer failed: %d\n", r);
			return NULL;
		}
		r = uv_timer_start(&tcp_server->timer, perfomance_print, 2000, 2000);
		if (r)
		{
			fprintf(stderr, "create timer failed: %d\n", r);
			return NULL;
		}
	}

	return tcp_server;
}

/*
 功能： 运行tcp服务实例
 参数：
 *  tcp_server: tcp服务实例，由tcp_server_create创建
 返回结果：无
 */

void tcp_server_run(tcp_server_t *tcp_server)
{
	uv_run(tcp_server->uvloop, 0);
}

/*
 功能： 关闭tcp服务实例并释放实例
 参数：
 *  tcp_server: tcp服务实例，由tcp_server_create创建
 返回结果：无
 */
void tcp_server_close(tcp_server_t *tcp_server)
{
//	uv_stop(tcp_server->uvloop);
	uv_loop_close(tcp_server->uvloop);
	free(tcp_server->uvloop);
	free(tcp_server);
}

/*
 功能： 异步发送数据
 参数：
 *  tcp_client: 客户端实例，连接成功时创建
 *  data :数据指针
 *  size ：数据大小
 返回结果：无
 说明：此函数调用会触发on_data回调函数
 */

void tcp_server_send_data(tcp_client_t *tcp_client, char *data, size_t size)
{
	tcp_server_t *tcp_server = tcp_client->server;
	tcp_server_on_error_func_t error_cb = tcp_server != NULL ? tcp_server->on_conn_error : tcp_client->on_conn_error;

	if (tcp_client->is_closed)
	{
		if (error_cb)
		{
			error_cb("connection have been closed", tcp_client);
		}
		return;
	}
	
	char *need_mem = (char *) calloc(1, sizeof (uv_write_t) + size + sizeof (tcp_send_data_cb_t));
	if (need_mem == NULL)
	{
		if (error_cb)
		{
			error_cb("allocate mem failed", tcp_client);
		}
		perror("allocate mem failed");
		return;
	}

	uv_write_t *write_req = (uv_write_t *) need_mem;
	char *send_data = (char *) ((char *) write_req + sizeof (*write_req));
	tcp_send_data_cb_t *p_data = (tcp_send_data_cb_t *) (send_data + size);

	memcpy(send_data, data, size);
	uv_buf_t buf = uv_buf_init(send_data, size);
	p_data->tcp_client = tcp_client;
	p_data->send_data = buf;
	write_req->data = p_data;
	uv_write(write_req, (uv_stream_t*) tcp_client->client, &buf, 1, on_client_write);
}

/*
 功能： 关闭客户端连接
 参数：
 *  tcp_client: 客户端实例，连接成功时创建
 返回结果：无
 说明：此函数调用会触发on_data回调函数
 */
void tcp_server_close_client(tcp_client_t *client)
{
	tcp_server_t *server = client->server;
	if (client->is_closed == 0)
	{
		uv_read_stop((uv_stream_t *) client->client);
		uv_close((uv_handle_t*) client, on_close_connection);
		server->conn_count--;
		client->is_closed = 1;
	}
	//	free(client->client);
	//	free(client);
}

/*
 功能： 回去连接的客户端地址
 参数：
 *  tcp_client: 客户端实例，连接成功时创建
 *  dst :返回结果
 *  len: dst缓存的大小
 返回结果：dst, 返回客户端地址
 */

char *tcp_server_get_client_addr(tcp_client_t *client, char *dst, size_t len)
{
	struct sockaddr_storage peername;
	int namelen = sizeof (peername);
	int r = uv_tcp_getpeername((uv_tcp_t*) (client->client), (struct sockaddr *) &peername, &namelen);
	if (r == 0)
	{
		if (peername.ss_family == AF_INET)
		{
			uv_ip4_name((const struct sockaddr_in*) &peername, dst, len);
		}
		else
		{
			uv_ip6_name((const struct sockaddr_in6*) &peername, dst, len);
		}
	}
	return dst;
}

/*
 功能： 设置服务端的最大连接数
 参数：
 *  tcp_server: 服务端实例，由tcp_server_create创建
 *  max :最大连接数
 返回结果：无
 */
void tcp_server_set_max_conn(tcp_server_t *tcp_server, int max)
{
	tcp_server->max_connections = max;
}
