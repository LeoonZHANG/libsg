/*
 * File:   newsimpletest1.c
 * Author: sundq
 *
 * Created on 2016-1-25, 20:46:38
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "../udp.h"

/*
 * Simple C Test Suite
 */
#define data_len 1024
static void udp_on_data(udp_socket_t *udp_handle, const struct sockaddr *peer, void *data, size_t len)
{
	char *p = (char *)data;
	printf("get data:%zd\n", len);
	usleep(1000);
//	udp_send_data(udp_handle, data, len, peer);
}

static void udp_on_send(udp_socket_t *conn, const struct sockaddr *peer, void *data, size_t len, int status)
{
	printf("send data ok:%zd\n", len);
}

void test1()
{
//	uv_udp_t send_socket;
//    uv_loop_t *loop = uv_default_loop();
	uv_udp_send_t send_req;
//
	struct sockaddr_in send_addr;
	uv_ip4_addr("127.0.0.1", 8000, &send_addr);
    udp_socket_t *udp_handle = udp_handle_client_create(udp_on_data, udp_on_send, NULL);
//	uv_udp_send(&send_req, &send_socket, &discover_msg, 1, send_addr, on_send);
	char data[data_len];
    udp_send_data(udp_handle, data, data_len, (const struct sockaddr *)&send_addr);
	udp_send_data_ip(udp_handle, data, data_len, "127.0.0.1", 8000);
	udp_handle_run(udp_handle);
//	return uv_run(loop,    );

}

int main(int argc, char** argv)
{
	test1();

	return (EXIT_SUCCESS);
}
