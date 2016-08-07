/*
 * net.h
 * Author: wangwei.
 * Common network util library.
 */

#ifndef LIBSG_NET_H
#define LIBSG_NET_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define IPv4_MAX_LEN 16
#define IPv6_MAX_LEN 46
#define IP_MAX_LEN IPv6_MAX_LEN

int sg_net_intranet_ip(void);

/*
 Get human readable ip address and port from struct sockaddr
 return 0: OK.
 return -1: error.
*/
//int sg_net_sockaddr_to_ip_port(const struct sockaddr *,  char *ipbuf,  int buflen, int* port);

/*
 Get binary ip address and port from struct sockaddr
 return 0: OK.
 return -1: error.
*/
//int sg_net_sockaddr_to_raw_ip_port(const struct sockaddr *, unsigned char *ipbuf, int *port);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_NET_H */
