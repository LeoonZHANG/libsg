#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <uv.h>
#include <sg/sg.h>

const char* uv_comm_get_tcp_ip_port(uv_tcp_t* uvclient, char* ipbuf, int buflen, int* port)
{
	struct sockaddr addr;
	int len = sizeof(addr);

	/* 先调用uv接口获取到struct sockaddr形态的地址 */
	int r = uv_tcp_getpeername(uvclient, &addr, &len);
	if(r == 0) {
	    /* 转换为可读风格的ip字符串和端口 */
		return uv_comm_get_ip_port(&addr, ipbuf, buflen, port);
	} else {
        printf("\n!!! [cmm] get client ip fails: %s\n", uv_strerror(r));
		return NULL;
	}
}

/* uv_comm_send_to_stream的结果处理回调函数 */
static void uv_comm_after_send_to_stream(uv_write_t* w, int status)
{
    /* status 为0时, send_to_stream成功, 其他情况处理失败 */
    if(status) {
        puts("\n!!! [cmm] uv_comm_after_send_to_stream(,-1) failed");
    }
    //释放在uv_comm_send_to_stream分配的内存
    free(w->data);
    free(w);
}

int uv_comm_send_to_stream(uv_stream_t* stream, void* data, unsigned int size)
{
    assert(stream && data);
    uv_buf_t buf;
    /* 构造传递给uv_write的buf */
    buf.base = (char*)data;
    buf.len = (size_t)size;

    /* 构造传递给结果回调函数uv_comm_after_send_to_streamuv_write_t */
    uv_write_t* w = (uv_write_t*) malloc(sizeof(uv_write_t));
    memset(w, 0, sizeof(uv_write_t));
    w->data = data;

    /* 调用uv_write写入数据 */
    return (uv_write(w, stream, &buf, 1, uv_comm_after_send_to_stream) == 0 ? 1 : 0);
}

/* libuv let you customize your memory alloc */
static void
uv_comm_on_alloc_buf(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
	buf->base = (char *)malloc(suggested_size);
	buf->len  = buf->base ? suggested_size : 0;
}

static void
uv_common_on_close_done(uv_handle_t *handle)
{
}
