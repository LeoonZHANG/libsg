/**
 * netcard.h
 * Network card information reader for Linux / OSX / Windows.
 */
#ifndef LIBSG_NETCARD_H
#define LIBSG_NETCARD_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* network card information */
struct sg_netcard_info {
    char name[256];          /* network card name such as 'eth0' */
    char mac[18];            /* mac address */
    bool is_static_ip;       /* static ip or dhcp */
    char lan_ipv4[16];       /* LAN ipv4, not WAN(internet) */
    char net_mask_ipv4[16];  /* subnet mask ipv4 */
    char broadcast_ipv4[16]; /* broadcast address ipv4 */
    char lan_ipv6[46];       /* LAN ipv6 */
    char net_mask_ipv6[46];  /* subnet mask ipv6 */
    char broadcast_ipv6[46]; /* broadcast address ipv6 */
    int  mtu;                /* max transfer unit, 1400 default in common */
};

struct sg_net_dns {
    int  dns_server_size;    /* valid dns_server size */
    struct name_server {
        char dns_server[16];
    } dns_server[10];        /* max 10 dns server */
};

typedef void (*sg_netcard_scan_cb_t)(struct sg_net_card_info *inf, void *ctx);

int sg_netcard_scan(sg_netcard_scan_cb_t cb, void *ctx, int merge_interfaces);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_NETCARD_H */
