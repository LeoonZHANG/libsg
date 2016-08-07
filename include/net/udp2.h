/*
 * udp2.h
 * Author: wangwei.
 * Udp client library.
 */

#ifndef LIBSG_UDP_CLIENT_H
#define LIBSG_UDP_CLIENT_H

#include <stdint.h>
#include <stddef.h>

enum sg_udp_client_mode {
    SGUDPCLIENTMODE_BLOCK = 0,
    SGUDPCLIENTMODE_NONBLOCK = 1,
};

typedef struct sg_udp_client_real sg_udp_client;

int sg_udp_client_init(void);

sg_udp_client *sg_udp_client_open(const char *server_addr, unsigned int server_port);

void sg_udp_client_set_mode(sg_udp_client *c, enum sg_udp_client_mode mode);

int sg_udp_client_set_send_timeout(sg_udp_client *c, int millisecond);

int sg_udp_client_set_recv_timeout(sg_udp_client *c, int millisecond);

int sg_udp_client_send(sg_udp_client *c, uint8_t *data, size_t size);

int sg_udp_client_recv(sg_udp_client *c, uint8_t *buf, size_t size);

void sg_udp_client_close(sg_udp_client *c);

void sg_udp_client_quit(void);

#endif /* LIBSG_UDP_CLIENT_H */