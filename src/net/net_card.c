#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sg/net/net_card.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/types.h>
#include <unistd.h>

#define get_inet_addr(version)                                                                                                 \
    if (if_addr->ifa_addr != NULL)                                                                                             \
        get_inet_addr_string(info.lan_ipv##version, if_addr->ifa_addr);                                                        \
                                                                                                                               \
    if (if_addr->ifa_netmask != NULL)                                                                                          \
        get_inet_addr_string(info.net_mask_ipv##version, if_addr->ifa_netmask);                                                \
                                                                                                                               \
    if (if_addr->ifa_dstaddr != NULL)                                                                                          \
        get_inet_addr_string(info.gateway_ipv##version, if_addr->ifa_dstaddr);

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

static size_t macos_get_if_mtu(const char* name)
{
    struct ifreq ifr = {0};
    strcpy(ifr.ifr_name, name);
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    ioctl(fd, SIOCGIFMTU, &ifr);

    close(fd);
    return ifr.ifr_mtu;
}

#if defined(__MACH__)
int sg_net_card_scan(sg_net_card_on_read_func_t callback, void* ctx)
{
    struct sg_net_card_info info = {0};
    bool base_on_ether = false;

    struct ifaddrs* if_addrs = NULL;
    if (getifaddrs(&if_addrs))
        return -1;

    for (struct ifaddrs* if_addr = if_addrs; if_addr; if_addr = if_addr->ifa_next) {

        if (strcmp(info.name, if_addr->ifa_name) != 0) {
            if (info.name[0] && base_on_ether)
                callback(&info, ctx);
            memset(&info, 0, sizeof(info));
            base_on_ether = false;
        }

        strncpy(info.name, if_addr->ifa_name, sizeof(info.name));

        switch (if_addr->ifa_addr->sa_family) {
        case AF_LINK:
            if (if_addr->ifa_addr != NULL) {
                struct sockaddr_dl* sdl = (struct sockaddr_dl*)if_addr->ifa_addr;
                unsigned char mac[6];
                if (6 == sdl->sdl_alen) {
                    memcpy(mac, LLADDR(sdl), sdl->sdl_alen);
                    sprintf(info.mac, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                    base_on_ether = true;
                }
            }
            info.mtu = macos_get_if_mtu(info.name);
            break;
        case AF_INET:
            get_inet_addr(4);
            break;
        case AF_INET6:
            get_inet_addr(6);
            break;
        default:
            continue;
        }
    }
    freeifaddrs(if_addrs);
    return 0;
}

#else

static void list_interfaces(int fd, void (*show)(int fd, const char* name))
{
    struct ifreq* ifreq;
    struct ifconf ifconf;
    char buf[16384];
    unsigned i;
    size_t len;

    ifconf.ifc_len = sizeof buf;
    ifconf.ifc_buf = buf;
    if (ioctl(fd, SIOCGIFCONF, &ifconf) != 0) {
        perror("ioctl(SIOCGIFCONF)");
        exit(EXIT_FAILURE);
    }

    ifreq = ifconf.ifc_req;
    for (i = 0; i < ifconf.ifc_len;) {
/* some systems have ifr_addr.sa_len and adjust the length that
 * way, but not mine. weird */
#ifndef linux
        len = IFNAMSIZ + ifreq->ifr_addr.sa_len;
#else
        len = sizeof *ifreq;
#endif
        if (show) {
            show(fd, ifreq->ifr_name);
        }
        else {
            printf("%s\n", ifreq->ifr_name);
        }
        ifreq = (struct ifreq*)((char*)ifreq + len);
        i += len;
    }
}

static void show_interface(int fd, const char* name)
{
    int family;
    struct ifreq ifreq;
    char host[128];
    memset(&ifreq, 0, sizeof ifreq);
    strncpy(ifreq.ifr_name, name, IFNAMSIZ);
    if (ioctl(fd, SIOCGIFADDR, &ifreq) != 0) {
        return; /* ignore */
    }
    switch (family = ifreq.ifr_addr.sa_family) {
    case AF_INET:
    case AF_INET6:
        getnameinfo(&ifreq.ifr_addr, sizeof ifreq.ifr_addr, host, sizeof host, 0, 0, NI_NUMERICHOST);
        break;
    default:
        sprintf(host, "unknown (family: %d)", family);
        return;
    }
    printf("%-24s%s\n", name, host);

    if (ioctl(fd, SIOCGIFMTU, &ifreq) != 0) {
        return; /* ignore */
    }
    else {
        printf("MTU:%d\n", ifreq.ifr_mtu);
    }
}

int sg_net_card_scan(sg_net_card_on_read_func_t callback, void* ctx)
{
    int s = socket(AF_INET, SOCK_DGRAM, 0);

    list_interfaces(s, show_interface);

    close(s);

    int s6 = socket(AF_INET6, SOCK_DGRAM, 0);

    // list_interfaces(s6, show_interface);

    close(s6);

    int error = ioctl(s, SIOCGIFMTU, &ifr);
    if (error)
        printf("SIOCGIFMTU error!\n");
    else {
        printf("MTU:%d\n", ifr.ifr_mtu);
    }

    error = ioctl(s, SIOCGIFHWADDR, &ifr);
    if (error)
        printf("SIOCGIFHWADDR error!\n");
    else {
        char* hw = ifr.ifr_hwaddr.sa_data;
        printf("mac address:%02x:%02x:%02x:%02x:%02x:%02x\n", hw[0], hw[1], hw[2], hw[3], hw[4], hw[5]);
    }

    error = ioctl(s, SIOCGIFADDR, &ifr);
    if (error)
        printf("SIOCGIFADDR error!\n");
    else {
        struct sockaddr_in* sin = (struct sockaddr_in*)&ifr.ifr_addr;
        char ip[16] = {0};
        inet_ntop(AF_INET, &sin_addr.s_addr, ip, sizeof(ip));
        printf("IP address: %s\n", ip);
    }

    return 0;
}
#endif
