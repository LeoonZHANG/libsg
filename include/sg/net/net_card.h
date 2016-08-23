/* 
 * 请参考github等开源实现
 * http://blog.csdn.net/weiyumingwww/article/details/17554461
 * Linux不同发行版网络配置文件位置可能不一样，请使用统一可靠的方式读取，最好通过系统API。
 * Windows下则要注意API的版本要兼容不同版本Windows。
 */
/**
 * net_card.h
 * Network card information reader for Linux / OSX / Windows.
 */

#ifndef LIBSG_NET_CARD_H
#define LIBSG_NET_CARD_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* network card information */
/* 定义错误的数组大小请修正 */
struct sg_net_card_info {
    char name[256];          /* network card name such as 'eth0' */
    char mac[18];            /* mac address */
    bool is_static_ip;       /* static ip or dhcp */
    char lan_ipv4[16];       /* LAN ipv4, not WAN(internet) */
    char lan_ipv6[46];       /* LAN ipv6 */
    char net_mask[16];       /* subnet mask */
    char gateway[16];        /* gateway server */
    int  mtu;                /* max transfer unit, 1400 default in common */
    int  dns_server_size;    /* valid dns_server size */
    struct name_server {
        char dns_server[16];
    } dns_server[10];        /* max 10 dns server */
};

typedef void (*sg_net_card_on_read_func_t)(struct sg_net_card_info inf, void *ctx);

int sg_net_card_scan(sg_net_card_on_read_func_t, void *ctx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_NET_CARD_H */