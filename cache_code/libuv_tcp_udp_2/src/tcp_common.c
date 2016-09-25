#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <assert.h>

#include <uv.h>



const char* cmm_get_ip_port(const struct sockaddr* addr, char* ipbuf, int buflen, int* port) {
    /* ����ipv4��ipv6, Ȼ��ֱ���ö�Ӧ�Ľӿڽ��д��� */
    switch (addr->sa_family) {
    case AF_INET: {
            const struct sockaddr_in* addrin = (const struct sockaddr_in*) addr;
			if(ipbuf) uv_inet_ntop(AF_INET, &(addrin->sin_addr), ipbuf, buflen);
            if(port)  *port = (int) ntohs(addrin->sin_port);
            break;
        }
    case AF_INET6: {
            const struct sockaddr_in6* addrin = (const struct sockaddr_in6*) addr;
			if(ipbuf) uv_inet_ntop(AF_INET6, &(addrin->sin6_addr), ipbuf, buflen);
            if(port)  *port = (int) ntohs(addrin->sin6_port);
            break;
        }
    default:
        if(port) *port = 0;
        return NULL;
    }
	return ipbuf ? ipbuf : NULL;
}

int cmm_get_raw_ip_port(const struct sockaddr* addr, unsigned char* ipbuf, int* port) {
    /* ����ipv4��ipv6, Ȼ��ֱ���ö�Ӧ�Ľӿڽ��д��� */
    switch (addr->sa_family) {
    case AF_INET: {
            const struct sockaddr_in* addrin = (const struct sockaddr_in*) addr;
            if(ipbuf) memcpy(ipbuf, &addrin->sin_addr, sizeof(addrin->sin_addr));
            if(port)  *port = (int) ntohs(addrin->sin_port);
            return sizeof(addrin->sin_addr); // 4
            break;
        }
    case AF_INET6: {
            const struct sockaddr_in6* addrin = (const struct sockaddr_in6*) addr;
            if(ipbuf) memcpy(ipbuf, &addrin->sin6_addr, sizeof(addrin->sin6_addr));
            if(port)  *port = (int) ntohs(addrin->sin6_port);
            return sizeof(addrin->sin6_addr); // 16
            break;
        }
    default:
        if(port) *port = 0;
        return 0;
    }
}

const char* cmm_get_tcp_ip_port(uv_tcp_t* uvclient, char* ipbuf, int buflen, int* port) {
	struct sockaddr addr;
	int len = sizeof(addr);

	/* �ȵ���uv�ӿڻ�ȡ��struct sockaddr��̬�ĵ�ַ */
	int r = uv_tcp_getpeername(uvclient, &addr, &len);
	if(r == 0) {
	    /* ת��Ϊ�ɶ�����ip�ַ����Ͷ˿� */
		return cmm_get_ip_port(&addr, ipbuf, buflen, port);
	} else {
        printf("\n!!! [cmm] get client ip fails: %s\n", uv_strerror(r));
		return NULL;
	}
}

/* cmm_send_to_stream�Ľ������ص����� */
static void cmm_after_send_to_stream(uv_write_t* w, int status) {
    /* status Ϊ0ʱ, send_to_stream�ɹ�, �����������ʧ�� */
    if(status) {
        puts("\n!!! [cmm] cmm_after_send_to_stream(,-1) failed");
    }
    //�ͷ���cmm_send_to_stream������ڴ�
    free(w->data);
    free(w);
}

int cmm_send_to_stream(uv_stream_t* stream, void* data, unsigned int size) {
    assert(stream && data);
    uv_buf_t buf;
    /* ���촫�ݸ�uv_write��buf */
    buf.base = (char*)data;
    buf.len = (size_t)size;

    /* ���촫�ݸ�����ص�����cmm_after_send_to_streamuv_write_t */
    uv_write_t* w = (uv_write_t*) malloc(sizeof(uv_write_t));
    memset(w, 0, sizeof(uv_write_t));
    w->data = data; 

    /* ����uv_writeд������ */
    return (uv_write(w, stream, &buf, 1, cmm_after_send_to_stream) == 0 ? 1 : 0);
}




//-----------------------------------------------------------------------------
// internal functions

void cmm__on_alloc_buf(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
	buf->base = (char*) malloc(suggested_size);
	buf->len  = buf->base ? suggested_size : 0;
}

#if defined(_WIN32) && !defined(__GNUC__)
#include <stdarg.h>
// Emulate snprintf() on Windows, _snprintf() doesn't zero-terminate the buffer on overflow...
int snprintf(char* buf, size_t len, const char* fmt, ...) {
	va_list ap;
	int n;

	va_start(ap, fmt);
	n = _vsprintf_p(buf, len, fmt, ap);
	va_end(ap);

	/* It's a sad fact of life that no one ever checks the return value of
	* snprintf(). Zero-terminating the buffer hopefully reduces the risk
	* of gaping security holes.
	*/
	if (n < 0)
		if (len > 0)
			buf[0] = '\0';

	return n;
}
#endif

