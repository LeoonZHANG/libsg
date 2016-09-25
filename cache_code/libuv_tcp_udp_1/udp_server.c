
#include "udp_server.h"


//异步发送数据回调使用

typedef struct udp_send_data_cb_s
{
	udp_socket_t *udp_handle;
	const struct sockaddr *addr; //发送数据的地址
	void *data; //发送的数据
	size_t len; //发送数据的大小
} udp_send_data_cb_t;

static void perfomance_print(uv_timer_t *handle)
{
	static long clktck = 0;
	if (0 == clktck)
	{
		if ((clktck = sysconf(_SC_CLK_TCK)) < 0)
		{
			perror("sysconf err");
			return;
		}
	}

	udp_socket_t *udp_handle = (udp_socket_t *) handle->data;
	struct tms tmsend = {0};
	struct tms tmsstart = udp_handle->tmsstart;
	times(&tmsend);
	double time_elapse = \
		(tmsend.tms_utime - tmsstart.tms_utime) / (double) clktck + //用户时间
		(tmsend.tms_stime - tmsstart.tms_stime) / (double) clktck + //内核时间
		(tmsend.tms_cutime - tmsstart.tms_cutime) / (double) clktck + //子进程用户时间
		(tmsend.tms_cstime - tmsstart.tms_cstime) / (double) clktck; //子进程内核时间

	double throughout = 0.0;
	if (time_elapse > 0.000001 && udp_handle->bytes_recv > 0)
	{
		throughout = (udp_handle->bytes_recv * 8) / (double) (1024 * 1024) / time_elapse; //Mbps
		fprintf(stderr, "udp server byte_recv=%zd, throughout=%.2fMbps\n", udp_handle->bytes_recv, throughout);
	}
}

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buff)
{
	buff->base = (char*) calloc(1, suggested_size);
	buff->len = suggested_size;
}

static void on_read(uv_udp_t *req, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags)
{
	udp_socket_t *udp_handle = (udp_socket_t *) req->data;

	if (nread > 0)
	{
		if (udp_handle->on_data)
		{
			udp_handle->bytes_recv += nread;
			udp_handle->on_data(udp_handle, addr, buf->base, nread);
		}
	}
	else if (nread == 0)
	{
		//fprintf(stderr, "on_read fire, but recv nothing");
	}	
	else
	{
		if (udp_handle->on_error)
		{
			udp_handle->on_error("Read error!", udp_handle);
		}
	}
	free(buf->base);
}

static void on_send(uv_udp_send_t* req, int status)
{
	udp_send_data_cb_t *p_data = (udp_send_data_cb_t *) req->data;
	udp_socket_t *udp_handle = (udp_socket_t *) p_data->udp_handle;
	if (udp_handle->on_send)
	{
		udp_handle->on_send(udp_handle, p_data->addr, p_data->data, p_data->len, status);
	}
	if (status == 0)
	{
		udp_handle->bytes_send += p_data->len;
	}
	free(req);
}

/*
 功能： 创建udp服务端实例
 参数：
 *  server_addr: 服务监听的地址
 *  server_port：服务监听的端口
 *  mode: 0 生产模式 1 开发模式(在此模式下回打印吞吐量的并发量
 *  on_data: 接收数据的回调函数
 *  on_error: 出错回调函数
 *  on_send: 异步发送数据完成后的回调函数
 返回结果：udp服务端实例
 */
udp_socket_t *udp_handle_server_create(const char *server_addr,
									   int server_port,
									   int mode,
									   udp_handle_on_data_func_t on_data,
									   udp_handle_on_send_func_t on_send,
									   udp_handle_on_error_func_t on_error)
{
	int r = 0;
	udp_socket_t *udp_handle = (udp_socket_t *) calloc(1, sizeof (udp_socket_t));
	udp_handle->on_data = on_data;
	udp_handle->on_send = on_send;
	udp_handle->on_error = on_error;
	udp_handle->uvloop = malloc(sizeof(uv_loop_t));
	
	uv_loop_init(udp_handle->uvloop);
	uv_udp_init(udp_handle->uvloop, &(udp_handle->udp));
	udp_handle->udp.data = udp_handle;

	if (server_addr != NULL)
	{
		struct sockaddr bind_addr = {0};
		if (strchr(server_addr, ':'))
		{
			uv_ip6_addr(server_addr, server_port, (struct sockaddr_in6 *) &bind_addr);
		}
		else
		{
			uv_ip4_addr(server_addr, server_port, (struct sockaddr_in *) &bind_addr);
		}
		uv_udp_bind(&(udp_handle->udp), (const struct sockaddr *) &bind_addr, 0);

		if (mode == 1)
		{
			udp_handle->timer.data = udp_handle;
			r = uv_timer_init(udp_handle->uvloop, &udp_handle->timer);
			if (r)
			{
				fprintf(stderr, "create timer failed: %d\n", r);
				return NULL;
			}
			r = uv_timer_start(&udp_handle->timer, perfomance_print, 2000, 2000);
			if (r)
			{
				fprintf(stderr, "create timer failed: %d\n", r);
				return NULL;
			}
		}
	}
	else
	{
		//this is a udp client
	}

	r = uv_udp_recv_start(&(udp_handle->udp), alloc_buffer, on_read);
	if (r)
	{
		fprintf(stderr, "start udp server failed:%d", r);
		return NULL;
	}

	return udp_handle;
}

/*
 功能： 设置udp实例的广播开关
 参数：
 *  udp_handle: udp实例
 *  on：1 on 0 off
 返回结果：0为成功 -1失败
 */
int udp_set_broadcast(udp_socket_t *udp_handle, int on)
{
	return uv_udp_set_broadcast(&udp_handle->udp, on);
}

//

/*
 功能： 发送数据到特定地址，地址用struct sockaddr 表示
 参数：
 *  udp_handle: udp实例
 *  data: 数据指针
 *  size：数据大小
 *  addr：目标地址
 返回结果：无
 说明：此函数调用会出发on_send回调函数调用
 */
void udp_send_data(udp_socket_t *udp_handle, char *data, size_t size, const struct sockaddr *addr)
{
	char *need_mem = (char *) calloc(1, sizeof (struct sockaddr) + sizeof (uv_udp_send_t) + size + sizeof (udp_send_data_cb_t));

	if (need_mem == NULL)
	{
		if (udp_handle->on_error)
		{
			udp_handle->on_error("allocate mem failed", udp_handle);
		}
		perror("allocate mem failed");
		return;
	}
	
	uv_udp_send_t *send_req = (uv_udp_send_t *) need_mem;
	struct sockaddr *send_addr = (struct sockaddr *) ((char *) send_req + sizeof (*send_req));
	char *send_data = (char *) send_addr + sizeof (*send_addr);
	udp_send_data_cb_t *p_data = (udp_send_data_cb_t *) (send_data + size);

	memcpy(send_addr, addr, sizeof (*send_addr));
	memcpy(send_data, data, size);
	uv_buf_t buf = {0};
	buf.base = send_data;
	buf.len = size;
	p_data->udp_handle = udp_handle;
	p_data->data = send_data;
	p_data->len = size;
	p_data->addr = send_addr;
	send_req->data = p_data;
	uv_udp_send(send_req, &(udp_handle->udp), &buf, 1, send_addr, on_send);
}

/*
 功能： 发送数据到特定ip和端口
 参数：
 *  udp_handle: udp实例
 *  data: 数据指针
 *  size：数据大小
 *  ip：目标地址
 *  port: 目标端口
 返回结果：无
 说明：此函数调用会出发on_send回调函数调用
 */
void udp_send_data_ip(udp_socket_t *udp_handle, char *data, size_t size, const char *ip, unsigned short port)
{
	struct sockaddr send_addr = {};
	if (strchr(ip, ':'))
	{
		uv_ip6_addr(ip, port, (struct sockaddr_in6 *) &send_addr);
	}
	else
	{
		uv_ip4_addr(ip, port, (struct sockaddr_in *) &send_addr);
	}
	udp_send_data(udp_handle, data, size, &send_addr);
}

/*
 功能： 运行udp实例
 参数：
 *  udp_handle: udp实例
 返回结果：无
 */

void udp_handle_run(udp_socket_t *udp_handle)
{
	uv_run(udp_handle->uvloop, UV_RUN_DEFAULT);
}

/*
 功能： 关闭并释放udp实例
 参数：
 *  udp_handle: udp实例
 返回结果：无
 */
void udp_handle_close(udp_socket_t *udp_handle)
{
    uv_udp_recv_stop(&udp_handle->udp);	
    uv_close((uv_handle_t*) &udp_handle->udp, NULL);
	uv_stop(udp_handle->uvloop);
	uv_loop_close(udp_handle->uvloop);
	free(udp_handle->uvloop);
	free(udp_handle);
}