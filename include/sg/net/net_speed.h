/**
 * net_speed.h
 * Net speed test using udp.
 */

#ifndef LIBSG_NET_SPEED_H
#define LIBSG_NET_SPEED_H

typedef struct sg_net_speed_client sg_net_speed_client_t;

typedef struct sg_net_speed_server sg_net_speed_server_t;

sg_net_speed_client_t *
sg_net_speed_client_start(const char *dst_host_addr,
                          const int dst_host_port,
                          int test_sec,
                          uint64_t *speed_Kbps,
                          uint64_t *speed_Kbps,
                          uint8_t *packet_loss_rate);

void sg_net_speed_client_stop(sg_net_speed_client_t *client);

sg_net_speed_server_t *
sg_net_speed_server_start(const char *addr,
                          const int port);

void sg_net_speed_server_stop(sg_net_speed_server_t *server);

#endif /* LIBSG_NET_SPEED_H */
