/*
 * File:   newsimpletest.c
 * Author: sundq
 *
 * Created on 2016-1-24, 18:11:25
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../tcp.h"

/*
 * Simple C Test Suite
 */

#define data_len 1204
static void on_open(tcp_client_t *tcp_client)
{
//    char data[data_len];
    char data[] = "world";
    tcp_client_send_data(tcp_client, data, data_len);
}

static void on_data(tcp_client_t *conn, void *data, size_t len)
{
    //char *p = (char *) data;
    printf("get data %zd %s\n", len, (char *)data);
	usleep(100);
//	tcp_client_send_data(conn, data, len);
}

static void on_send(tcp_client_t *conn, int status, void *data, size_t len)
{
    char *p = (char *) data;
    if (status == 0)
    {
        printf("data send ok:%zd\n", len);
    }
    else
    {
        printf("data send failed\n");
    }
}

void test1()
{
    printf("newsimpletest test 1\n");
    tcp_client_t *c = tcp_client_create("127.0.0.1", 7000, on_open, on_data, NULL, NULL, on_send);
    tcp_client_run();
}

int main(int argc, char** argv)
{
    test1();
}
