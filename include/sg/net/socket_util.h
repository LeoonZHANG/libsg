/**
 * socket_util.h
 * Common socket util library.
 */

#ifndef LIBSG_SOCKET_UTIL_H
#define LIBSG_SOCKET_UTIL_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

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

#endif /* LIBSG_SOCKET_UTIL_H */
