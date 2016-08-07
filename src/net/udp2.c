/*
 * udp_client.c
 * Author: wangwei.
 * Udp client library.
 */

#include <stdio.h>
#include <string.h>
#include "../../include/sys/os.h"
#if defined(OS_WIN)
# include <Windef.h> /* MAKEWORD */
# include <winsock2.h>
# pragma comment(lib, "ws2_32.lib")
#else
# include <sys/types.h>
# include <sys/time.h> /* timeval */
# include <netinet/in.h>
# include <sys/socket.h>
# include <errno.h>
# include <stdlib.h>
# include <arpa/inet.h>
# include <fcntl.h> /* fcntl */
#endif
#include "../../include/net/udp2.h"
#include "../../include/util/assert.h"


struct sg_udp_client_real {
    struct sockaddr_in addr;
#if defined(OS_WIN)
    SOCKET fd;
#else
    int fd;
#endif
};

enum timeout_dst {
    TIMEOUTDST_RECV = 0,
    TIMEOUTDST_SEND = 1
};

int udp_client_set_timeout(sg_udp_client *c, enum timeout_dst dst, int millisecond);

int sg_udp_client_init(void)
{
#if defined(OS_WIN)
    int retval;
    WORD version_request;
    WSAData wsa_data;

    /* Initialize network library against given version. */
    version_request = MAKEWORD(2 /* lowbyte */, 2 /* highbyte */);
    retval = WSAStartup(versionRequest, &wsaData);
    if (retval != 0 ) {
        printf("ERROR!");
        return -1;
    }

    /*
     * Confirm that the WinSock DLL supports 2.2.
     * Note that if the DLL supports versions greater
     * than 2.2 in addition to 2.2, it will still return
     * 2.2 in wVersion since that is the version we requested.
     */
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        printf("Could not find a usable version of Winsock.dll\n"); /* Could not find a usable WinSock DLL. */
        WSACleanup();
        return -1;
    }

    return 0; /* The Winsock 2.2 dll was found. */
#else
    return 0;
#endif
}

sg_udp_client *sg_udp_client_open(const char *server_addr, unsigned int server_port)
{
    sg_udp_client *c;

    c = (sg_udp_client *)malloc(sizeof(sg_udp_client));
    if (!c) {
        /**/
        return NULL;
    }

    /* Protocol == 0 means default, IPPROTO_UDP is the default protocol when socket type is SOCK_DGRAM. */
    c->fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (c->fd == -1) {
        /**/
        return NULL;
    }

    c->addr.sin_family = AF_INET;
    c->addr.sin_port = htons(server_port);
    c->addr.sin_addr.s_addr = inet_addr(server_addr);

    return c;
}

void sg_udp_client_set_mode(sg_udp_client *c, enum sg_udp_client_mode mode)
{
#if defined(OS_WIN)
    int m = (mode == SGUDPCLIENTMODE_BLOCK) ? 0 : 1; /* 0: block， 1: non-block */
    int err;

    err = ioctlsocket(c->fd, FIONBIO, (u_long FAR *)&m);
    if (err == SOCKET_ERROR) {
         /*WSAGetLastError()*/
         return -1;
    } else
        return 0;
#else
    int err;

    /*
        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    */
    /*err = fcntl(c->fd, F_SETFL, (mode == SGUDPCLIENTMODE_BLOCK) ? O_BLOCK : O_NONBLOCK);*/
    err = 0;
    if (err != 0) {
        //errno()
        return -1;
    } else
        return 0;
#endif
}

int udp_client_set_timeout(sg_udp_client *c, enum timeout_dst dst, int milliseconds)
{
#if defined(OS_WIN)
    int timeout = milliseconds;
    int retval;

    retval = setsockopt(serSocket, SOL_SOCKET,
        (opt == TIMEOUTDST_RECV) ? SO_RCVTIMEO : SO_SNDTIMEO, (char *)&timeout, sizeof(int));
    if (retval != 0)
        return -1;
    else
        return 0;
#else
    struct timeval timeout;
    int retval;

    timeout.tv_sec = milliseconds / 1000;
    timeout.tv_usec = milliseconds % 1000 * 1000;

    retval = setsockopt(c->fd, SOL_SOCKET, (dst == TIMEOUTDST_RECV) ? SO_RCVTIMEO : SO_SNDTIMEO,
                        (char *)&timeout, sizeof(struct timeval));
    if (retval != 0)
        return -1;
    else
        return 0;
#endif
}

int sg_udp_client_set_send_timeout(sg_udp_client *c, int millisecond)
{
    return sg_udp_client_set_timeout(c, TIMEOUTDST_SEND, millisecond);
}

int sg_udp_client_set_recv_timeout(sg_udp_client *c, int millisecond)
{
    return sg_udp_client_set_timeout(c, TIMEOUTDST_RECV, millisecond);
}

int sg_udp_client_send(sg_udp_client *c, uint8_t *data, size_t size)
{
    int retval;

    errno = 0;
    retval = sendto(c->fd, data, size, 0, (struct sockaddr *)&c->addr, sizeof(c->addr));
    if (retval < 0) {
        fprintf(stderr, "%s", strerror(errno));
    }
    return retval;
}

int sg_udp_client_recv(sg_udp_client *c, uint8_t *buf, size_t size)
{
    int retval;
    int addr_len;

    addr_len = sizeof(struct sockaddr_in);
    errno = 0;
    retval = recvfrom(c->fd, buf, size, 0, (struct sockaddr *)&c->addr, &addr_len);
    if (retval < 0) {
        fprintf(stderr, "%s", strerror(errno));
        return -1;
    }
    return retval;
}

void sg_udp_client_close(sg_udp_client *c)
{
    sg_assert(c);

    if (c->fd)
#if defined(OS_WIN)
        closesocket(c->fd);
#else
        close(c->fd);
#endif
    free(c);
}

void sg_udp_client_quit(void)
{
#if defined(OS_WIN)
    WSACleanup();
#endif
}