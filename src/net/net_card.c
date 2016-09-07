#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sg/net/net_card.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#ifdef __MACH__
#include <net/if_dl.h>
#include <sys/sockio.h>
#else
#include <netpacket/packet.h>
#endif

#ifndef WIN32

#define get_inet_addr(version)                                                                                                 \
    if (if_addr->ifa_addr != NULL)                                                                                             \
        get_inet_addr_string(if_info_set[actual_if].lan_ipv##version, if_addr->ifa_addr);                                      \
                                                                                                                               \
    if (if_addr->ifa_netmask != NULL)                                                                                          \
        get_inet_addr_string(if_info_set[actual_if].net_mask_ipv##version, if_addr->ifa_netmask);                              \
                                                                                                                               \
    if (if_addr->ifa_dstaddr != NULL)                                                                                          \
        get_inet_addr_string(if_info_set[actual_if].broadcast_ipv##version, if_addr->ifa_dstaddr);

static void get_inet_addr_string(char* string, struct sockaddr* addr)
{
    char buf[INET6_ADDRSTRLEN];
    if (addr->sa_family == AF_INET) {
        void* tmp = &((struct sockaddr_in*)addr)->sin_addr;
        inet_ntop(addr->sa_family, tmp, buf, sizeof(buf));
        sprintf(string, "%s", buf);
    }
    else {
        void* tmp = &((struct sockaddr_in6*)addr)->sin6_addr;
        inet_ntop(addr->sa_family, tmp, buf, sizeof(buf));
        sprintf(string, "%s", buf);
    }
}

static size_t ioctl_get_if_mtu(const char* name)
{
    struct ifreq ifr = {0};
    strcpy(ifr.ifr_name, name);
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    ioctl(fd, SIOCGIFMTU, &ifr);

    close(fd);
    return ifr.ifr_mtu;
}

size_t merge_interfaces_info(struct sg_net_card_info* if_info_set, size_t actual_if)
{
    size_t merged_count = 0;
    for (size_t i = 0; i < actual_if; ++i) {
        int duplicated = 0;
        for (size_t j = 0; j < actual_if; ++j) {
            if (i == j || strcmp(if_info_set[i].name, if_info_set[j].name))
                continue;
            if (i > j) {
                duplicated = 1;
                break;
            }

            if (!if_info_set[i].mac[0] && if_info_set[j].mac[0])
                memcpy(if_info_set[i].mac, if_info_set[j].mac, sizeof(if_info_set[i].mac));
            if (!if_info_set[i].lan_ipv4[0] && if_info_set[j].lan_ipv4[0])
                memcpy(if_info_set[i].lan_ipv4, if_info_set[j].lan_ipv4, sizeof(if_info_set[i].lan_ipv4));
            if (!if_info_set[i].net_mask_ipv4[0] && if_info_set[j].net_mask_ipv4[0])
                memcpy(if_info_set[i].net_mask_ipv4, if_info_set[j].net_mask_ipv4, sizeof(if_info_set[i].net_mask_ipv4));
            if (!if_info_set[i].broadcast_ipv4[0] && if_info_set[j].broadcast_ipv4[0])
                memcpy(if_info_set[i].broadcast_ipv4, if_info_set[j].broadcast_ipv4, sizeof(if_info_set[i].broadcast_ipv4));
            if (!if_info_set[i].lan_ipv6[0] && if_info_set[j].lan_ipv6[0])
                memcpy(if_info_set[i].lan_ipv6, if_info_set[j].lan_ipv6, sizeof(if_info_set[i].lan_ipv6));
            if (!if_info_set[i].net_mask_ipv6[0] && if_info_set[j].net_mask_ipv6[0])
                memcpy(if_info_set[i].net_mask_ipv6, if_info_set[j].net_mask_ipv6, sizeof(if_info_set[i].net_mask_ipv6));
            if (!if_info_set[i].broadcast_ipv6[0] && if_info_set[j].broadcast_ipv6[0])
                memcpy(if_info_set[i].broadcast_ipv6, if_info_set[j].broadcast_ipv6, sizeof(if_info_set[i].broadcast_ipv6));
            if (!if_info_set[i].mtu && if_info_set[j].mtu)
                if_info_set[i].mtu = if_info_set[j].mtu;
        }
        if (!duplicated) {
            if (i != merged_count)
                if_info_set[merged_count] = if_info_set[i];
            ++merged_count;
        }
    }
    actual_if = merged_count;
}

int sg_net_card_scan(sg_net_card_on_read_func_t callback, void* ctx, int merge_interfaces)
{
    struct ifaddrs* if_addrs = NULL;
    if (getifaddrs(&if_addrs))
        return -1;

    size_t max_if = 0;
    for (struct ifaddrs* if_addr = if_addrs; if_addr; if_addr = if_addr->ifa_next)
        ++max_if;
    struct sg_net_card_info *if_info_set = calloc(max_if, sizeof(struct sg_net_card_info));

    size_t actual_if = 0;
    for (struct ifaddrs* if_addr = if_addrs; if_addr; if_addr = if_addr->ifa_next) {

        strncpy(if_info_set[actual_if].name, if_addr->ifa_name, sizeof(if_info_set[actual_if].name));

        switch (if_addr->ifa_addr->sa_family) {
#ifdef __MACH__
        case AF_LINK:
#else
        case AF_PACKET:
#endif
            if (if_addr->ifa_addr != NULL) {
                unsigned char mac[6];
#ifdef __MACH__
                struct sockaddr_dl* sdl = (struct sockaddr_dl*)if_addr->ifa_addr;
                if (6 == sdl->sdl_alen) {
                    memcpy(mac, LLADDR(sdl), sdl->sdl_alen);
#else
                struct sockaddr_ll* sdl = (struct sockaddr_ll*)if_addr->ifa_addr;
                if (6 == sdl->sll_halen) {
                    memcpy(mac, sdl->sll_addr, sdl->sll_halen);
#endif
                    sprintf(if_info_set[actual_if].mac, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                }
            }
            if_info_set[actual_if].mtu = ioctl_get_if_mtu(if_info_set[actual_if].name);
            ++actual_if;
            break;
        case AF_INET:
            get_inet_addr(4);
            ++actual_if;
            break;
        case AF_INET6:
            get_inet_addr(6);
            ++actual_if;
            break;
        default:
            continue;
        }
    }
    freeifaddrs(if_addrs);

    if (merge_interfaces)
        actual_if = merge_interfaces_info(if_info_set, actual_if);

    for (size_t i = 0; i < actual_if; ++i)
        callback(&if_info_set[i], ctx);

    free(if_info_set);
    return 0;
}

#else

#endif
