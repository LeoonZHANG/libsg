/**
 * net.c
 * Common network util library.
 */

#include <ifaddrs.h>
#include <arpa/inet.h>
#include <stddef.h>
#include "../../include/net/net.h"

int sg_net_intranet_ip(void)
{
    #define IP_LEN_MAX INET6_ADDRSTRLEN
    void *swap_addr;
    char ip[IP_LEN_MAX];
    struct ifaddrs *addr;

    getifaddrs(&addr);
    while (addr != NULL) {
        if (addr->ifa_addr->sa_family == AF_INET) { /* IPv4. */
            swap_addr = &((struct sockaddr_in *)addr->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, swap_addr, ip, IP_LEN_MAX);
            //printf("%s IPv4 address: %s.\n", addr->ifa_name, ip);
        } else if (addr->ifa_addr->sa_family == AF_INET6) { /* IPv6. */
            swap_addr = &((struct sockaddr_in *)addr->ifa_addr)->sin_addr;
            inet_ntop(AF_INET6, swap_addr, ip, IP_LEN_MAX);
            //printf("%s IPv6 address: %s.\n", addr->ifa_name, ip);
        }
        addr = addr->ifa_next;
    }

    return 0;
}

/*
int is_ipv4(const char *ip)
{
    return 0;
}

int is_ipv6(const char *ip)
{
    return 0;
}

int is_ip_valid(const char *ip)
{
    char reg[] = "((?:(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d)))\.) {3}(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d))))";
    return 0;
}

int is_ip_internet(const char *ip)
{
    return 0;
}*/
#if 0
int sg_net_sockaddr_to_ip_port(const struct sockaddr *addr, char *ipbuf, int buflen, int *port)
{
	switch (addr->sa_family) {
	case AF_INET: /* IPv4 */
		const struct sockaddr_in *addrin = (const struct sockaddr_in *)addr;
		if(ipbuf)
			inet_ntop(AF_INET, &(addrin->sin_addr), ipbuf, buflen);
		if(port)
			*port = (int)ntohs(addrin->sin_port);
		break;
	case AF_INET6: /* IPv6 */
		const struct sockaddr_in6 *addrin = (const struct sockaddr_in6 *)addr;
		if(ipbuf)
			inet_ntop(AF_INET6, &(addrin->sin6_addr), ipbuf, buflen);
		if(port)
			*port = (int)ntohs(addrin->sin6_port);
		break;
	default:
		if(port)
			*port = 0;
		break;
	}

	return 0;
}

int sg_net_sockaddr_to_raw_ip_port(const struct sockaddr *addr, unsigned char *ipbuf, int *port)
{
	SG_ASSERT(addr);

	switch (addr->sa_family) {
	case AF_INET: /* IPv4 */
		const struct sockaddr_in *addrin = (const struct sockaddr_in *)addr;
		if (ipbuf)
			memcpy(ipbuf, &addrin->sin_addr, sizeof(addrin->sin_addr));
		if (port)
			*port = (int)ntohs(addrin->sin_port);
		break;
	case AF_INET6: /* IPv6 */
		const struct sockaddr_in6 *addrin = (const struct sockaddr_in6 *)addr;
		if(ipbuf)
			memcpy(ipbuf, &addrin->sin6_addr, sizeof(addrin->sin6_addr));
		if(port)
			*port = (int)ntohs(addrin->sin6_port);
		break;
	default:
		if(port)
			*port = 0;
		break;
	}

	return 0;
}
#endif
