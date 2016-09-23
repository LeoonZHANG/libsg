#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sg/sg.h>
#include <sg/net/net_card.h>

#ifdef SG_OS_WINDOWS
# include <winsock2.h>
# include <WS2tcpip.h>
# include <iphlpapi.h>
# pragma comment(lib, "iphlpapi.lib")
#else
# include <arpa/inet.h>
# include <errno.h>
# include <ifaddrs.h>
# include <net/if.h>
# include <netdb.h>
# include <netinet/in.h>
# include <sys/ioctl.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <unistd.h>
#endif

#ifdef SG_OS_MACOS
# include <net/if_dl.h>
# include <sys/sockio.h>
#else
# include <netpacket/packet.h>
#endif

#ifdef SG_OS_WINDOWS

static int is_vista_above()
{
    OSVERSIONINFO osvi;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osvi);
    return (osvi.dwMajorVersion > 5);
}

static size_t get_address_prefix(PIP_ADAPTER_ADDRESSES pCurrAddresses, PIP_ADAPTER_UNICAST_ADDRESS address, int family)
{
    // XP has no OnLinkPrefixLength field.
    size_t prefix = 0;
    if (is_vista_above()) {
        prefix = address->OnLinkPrefixLength;
    }
    else {
        // Prior to Windows Vista the FirstPrefix pointed to the list with
        // single prefix for each IP address assigned to the adapter.
        // Order of FirstPrefix does not match order of FirstUnicastAddress,
        // so we need to find corresponding prefix.
        for (IP_ADAPTER_PREFIX* p = pCurrAddresses->FirstPrefix; p; p = p->Next) {
            if (p->Address.lpSockaddr->sa_family == family) {
                prefix = max(prefix, p->PrefixLength);
            }
        }
    }
    return prefix;
}

int sg_net_card_scan(sg_net_card_on_read_func_t callback, void* ctx, int merge_interfaces)
{
    // Set the flags to pass to GetAdaptersAddresses
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
    // default to unspecified address family (both)
    ULONG family = AF_UNSPEC;

    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    ULONG outBufLen = 0;

    outBufLen = sizeof(IP_ADAPTER_ADDRESSES);
    pAddresses = (IP_ADAPTER_ADDRESSES *)malloc(outBufLen);

    // Make an initial call to GetAdaptersAddresses to get the
    // size needed into the outBufLen variable
    if (GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(pAddresses);
        pAddresses = (IP_ADAPTER_ADDRESSES *)malloc(outBufLen);
    }

    if (pAddresses == NULL) {
        return -1;
    }

    // Make a second call to GetAdapters Addresses to get the
    DWORD dwRetVal = GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);
    if (dwRetVal != NO_ERROR) {
        free(pAddresses);
        return -1;
    }

    // If successful, output some information from the data we received
    size_t max_if = 0;
    for (PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses; pCurrAddresses; pCurrAddresses = pCurrAddresses->Next)
        ++max_if;

    struct sg_net_card_info *if_info_set = calloc(max_if, sizeof(struct sg_net_card_info));
    size_t actual_if = 0;
    for (PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses; pCurrAddresses; pCurrAddresses = pCurrAddresses->Next) {
        if (pCurrAddresses->IfType != IF_TYPE_ETHERNET_CSMACD && pCurrAddresses->IfType != IF_TYPE_IEEE80211)
            continue;

        // FIXME: Should used Description instead of UUID style AdapterName?
        strcpy(if_info_set[actual_if].name, pCurrAddresses->AdapterName);
        // hard-coded mac length as 6 because of ethernet specification
        sprintf(if_info_set[actual_if].mac, "%02x:%02x:%02x:%02x:%02x:%02x",
            pCurrAddresses->PhysicalAddress[0], pCurrAddresses->PhysicalAddress[1],
            pCurrAddresses->PhysicalAddress[2], pCurrAddresses->PhysicalAddress[3],
            pCurrAddresses->PhysicalAddress[4], pCurrAddresses->PhysicalAddress[5]);
        if_info_set[actual_if].mtu = pCurrAddresses->Mtu;
        if_info_set[actual_if].is_static_ip = !pCurrAddresses->Flags & IP_ADAPTER_DHCP_ENABLED;
        LPSOCKADDR lpaddr = pCurrAddresses->FirstUnicastAddress->Address.lpSockaddr;
        DWORD addrlen = pCurrAddresses->FirstUnicastAddress->Address.iSockaddrLength;


        for (PIP_ADAPTER_UNICAST_ADDRESS addr = pCurrAddresses->FirstUnicastAddress; addr; addr = addr->Next) {
            if (addr->Address.lpSockaddr->sa_family == AF_INET) {
                struct sockaddr_in *sa_in = (struct sockaddr_in *)addr->Address.lpSockaddr;
                inet_ntop(AF_INET, &(sa_in->sin_addr), if_info_set[actual_if].lan_ipv4, sizeof(if_info_set[actual_if].lan_ipv4));

                size_t prefix = get_address_prefix(pCurrAddresses, addr, AF_INET);
                ULONG mask = 0;
                ConvertLengthToIpv4Mask(prefix, &mask);
                sa_in->sin_addr.s_addr |= ~mask;
                inet_ntop(AF_INET, &(sa_in->sin_addr), if_info_set[actual_if].broadcast_ipv4, sizeof(if_info_set[actual_if].broadcast_ipv4));
                sa_in->sin_addr.s_addr = mask;
                inet_ntop(AF_INET, &(sa_in->sin_addr), if_info_set[actual_if].net_mask_ipv4, sizeof(if_info_set[actual_if].net_mask_ipv4));
            }
            else if (addr->Address.lpSockaddr->sa_family == AF_INET6) {
                struct sockaddr_in6 *sa_in6 = (struct sockaddr_in6 *)addr->Address.lpSockaddr;
                inet_ntop(AF_INET6, &(sa_in6->sin6_addr), if_info_set[actual_if].lan_ipv6, sizeof(if_info_set[actual_if].lan_ipv6));

                size_t prefix = get_address_prefix(pCurrAddresses, addr, AF_INET6);
                size_t suffixLength = 128 - prefix;
                for (size_t i = 0, suffix = suffixLength; suffix && i < 16; ++i) {
                    size_t suffix_byte = min(8, suffix);
                    unsigned char mask_byte = ~((1 << suffix_byte) - 1);
                    sa_in6->sin6_addr.u.Byte[16 - i - 1] |= ~mask_byte;
                    suffix -= suffix_byte;
                }
                inet_ntop(AF_INET6, &(sa_in6->sin6_addr), if_info_set[actual_if].broadcast_ipv6, sizeof(if_info_set[actual_if].broadcast_ipv6));
                for (size_t i = 0, suffix = suffixLength; i < 16; ++i) {
                    size_t suffix_byte = min(8, suffix);
                    unsigned char mask_byte = ~((1 << suffix_byte) - 1);
                    sa_in6->sin6_addr.u.Byte[16 - i - 1] = mask_byte;
                    suffix -= suffix_byte;
                }
                inet_ntop(AF_INET6, &(sa_in6->sin6_addr), if_info_set[actual_if].net_mask_ipv6, sizeof(if_info_set[actual_if].net_mask_ipv6));
            }
        }

        ++actual_if;
    }

    free(pAddresses);

    for (size_t i = 0; i < actual_if; ++i)
        callback(&if_info_set[i], ctx);
    return 0;
}

#else

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
#ifdef SG_OS_MACOS
        case AF_LINK:
#else
        case AF_PACKET:
#endif
            if (if_addr->ifa_addr != NULL) {
                unsigned char mac[6];
#ifdef SG_OS_MACOS
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

#endif
