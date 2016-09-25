#include <signal.h>
#include "../tcp_server.h"
#include "../udp_server.h"

void on_connect(tcp_client_t *conn)
{
	char peer[64] = {0};
	tcp_server_get_client_addr(conn, peer, 64);
	printf("connection comming from %s\n", peer);
}

void on_data(tcp_client_t *conn, void *data, size_t len)
{
//	printf("get data %zd\n", len);
	tcp_server_send_data(conn, data, len);
}

void on_send(tcp_client_t *conn, int status, void *data, size_t len)
{
	if (status == 0)
	{
//		printf("data send ok:%zd\n", len);
	}
	else
	{
		printf("data send failed:%zd\n", len);
	}

	//	tcp_server_close(conn->server);

}

static void on_close(tcp_client_t *conn)
{
	fprintf(stderr, "connect close.\n");
}

void on_failed(const char *errmsg, tcp_client_t *client)
{
	fprintf(stderr, "%s\n", errmsg);
	perror("read failed");
}

void udp_on_data(udp_socket_t *udp_handle, const struct sockaddr *peer, void *data, size_t len)
{
	char *p = (char *) data;
	//	p[len] = 0;
//	fprintf(stderr, "get data:%zd\n", len);
	udp_send_data(udp_handle, data, len, peer);
}

void udp_on_send(udp_socket_t *conn, const struct sockaddr *peer, void *data, size_t len, int status)
{
//	fprintf(stderr, "send data ok:%zd\n", len);
}

int main(void)
{
	tcp_server_t* s = tcp_server_create("0.0.0.0", 7000, 128, 1, on_connect, on_close, on_failed, on_data, on_send);
	//   tcp_server_set_max_conn(s, 0);
	udp_socket_t* u_s = udp_handle_server_create("0.0.0.0", 8000, 1, udp_on_data, udp_on_send, NULL);
	printf("ok\n");
    signal(SIGPIPE,SIG_IGN);
	if (s != NULL && u_s != NULL)
	{
		tcp_server_run(s);
	    udp_handle_run(u_s);	
	}
}