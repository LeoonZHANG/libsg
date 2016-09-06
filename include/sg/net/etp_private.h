/**
 * etp_private.h
 *
 * This is for etp client/server, other module should not include this file.
 */

#ifndef LIBSG_ETP_PRIVATE_H
#define LIBSG_ETP_PRIVATE_H

#if defined(linux) || defined(__linux) || defined(__linux__)
#   ifndef PLATFORM_LINUX
#       define PLATFORM_LINUX
#   endif
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64)
#   ifndef PLATFORM_WINDOWS
#       define PLATFORM_WINDOWS
#   endif
#elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
#   ifndef PLATFORM_MACOS
#       define PLATFORM_MACOS
#   endif
#else
#   error Unsupported platform.
#endif

#if defined(PLATFORM_WINDOWS)
#   include <winsock2.h>
#   include <windows.h>
#   pragma comment(lib ,"ws2_32.lib")
#   pragma comment(lib, "psapi.lib")
#   pragma comment(lib, "Iphlpapi.lib")
#   pragma comment(lib, "userenv.lib")
#elif defined(PLATFORM_LINUX)
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <arpa/inet.h>
#   include <netinet/in.h>
#   include <netinet/tcp.h>
#   include <sys/epoll.h>
#   include <sys/time.h>
#elif defined(PLATFORM_MACOS) || defined(PLATFORM_BSD)
#   include <sys/event.h>
#   include <sys/socket.h>
#   include <arpa/inet.h>
#   include <sys/types.h>
#   include <sys/event.h>
#   include <sys/time.h>
#endif

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef unsigned char bool_t;
#define true    1
#define false   0

#define OK      0
#define ERROR (-1)

#define LOG_LVL_E 'E'
#define LOG_LVL_W 'W'
#define LOG_LVL_I 'I'
#define LOG_LVL_D 0
#if defined(PLATFORM_WINDOWS)
/* FIXME: change variable argument macros definition if needed. */
#define LOG(lvl, fmt, ...) if (lvl) { printf("%c: %s():%d " fmt "\n", lvl, __FUNCTION__, __LINE__, ##__VA_ARGS__); } else {}
#define LOG_E(prmpt, ...) LOG(LOG_LVL_E, prmpt, ##__VA_ARGS__)
#define LOG_W(prmpt, ...) LOG(LOG_LVL_W, prmpt, ##__VA_ARGS__)
#define LOG_I(prmpt, ...) LOG(LOG_LVL_I, prmpt, ##__VA_ARGS__)
#define LOG_D(prmpt, ...) LOG(LOG_LVL_D, prmpt, ##__VA_ARGS__)
#elif defined(PLATFORM_LINUX)
#define LOG(lvl, fmt, ...) if (lvl) { printf("%c: %s():%d " fmt "\n", lvl, __FUNCTION__, __LINE__, ##__VA_ARGS__); } else {}
#define LOG_E(prmpt, ...) LOG(LOG_LVL_E, prmpt, ##__VA_ARGS__)
#define LOG_W(prmpt, ...) LOG(LOG_LVL_W, prmpt, ##__VA_ARGS__)
#define LOG_I(prmpt, ...) LOG(LOG_LVL_I, prmpt, ##__VA_ARGS__)
#define LOG_D(prmpt, ...) LOG(LOG_LVL_D, prmpt, ##__VA_ARGS__)
#elif defined(PLATFORM_MACOS) || defined(PLATFORM_BSD)
/* FIXME: change variable argument macros definition if needed. */
#define LOG(lvl, fmt, ...) if (lvl) { printf("%c: %s():%d " fmt "\n", lvl, __FUNCTION__, __LINE__, ##__VA_ARGS__); } else {}
#define LOG_E(prmpt, ...) LOG(LOG_LVL_E, prmpt, ##__VA_ARGS__)
#define LOG_W(prmpt, ...) LOG(LOG_LVL_W, prmpt, ##__VA_ARGS__)
#define LOG_I(prmpt, ...) LOG(LOG_LVL_I, prmpt, ##__VA_ARGS__)
#define LOG_D(prmpt, ...) LOG(LOG_LVL_D, prmpt, ##__VA_ARGS__)
#else
/* FIXME: change variable argument macros definition if needed. */
#define LOG(lvl, fmt, ...) if (lvl) { printf("%c: %s():%d " fmt "\n", lvl, __FUNCTION__, __LINE__, ##__VA_ARGS__); } else {}
#define LOG_E(prmpt, ...) LOG(LOG_LVL_E, prmpt, ##__VA_ARGS__)
#define LOG_W(prmpt, ...) LOG(LOG_LVL_W, prmpt, ##__VA_ARGS__)
#define LOG_I(prmpt, ...) LOG(LOG_LVL_I, prmpt, ##__VA_ARGS__)
#define LOG_D(prmpt, ...) LOG(LOG_LVL_D, prmpt, ##__VA_ARGS__)
#endif

#define SG_ASSERT(exp, prmpt)          if (exp) {} else { LOG_W(prmpt); return; }
#define SG_ASSERT_RET(exp, prmpt, ret) if (exp) {} else { LOG_W(prmpt); return(ret); }
#define SG_ASSERT_BRK(exp, prmpt)      if (exp) {} else { LOG_W(prmpt); break; }

#define SG_CALLBACK(func, ...) if ((func) != NULL) { (func)(__VA_ARGS__); } else {LOG_D("%s is not registed", #func);}



/******************************************************************************/

/* callback for libuv udp */
void on_uv_alloc_buffer(uv_handle_t* handle, size_t size, uv_buf_t* buf);
/* callback for libuv udp */
void on_uv_close_done(uv_handle_t* handle);


/******************************************************************************/
typedef struct sg_etp_real sg_etp_session_t;

sg_etp_session_t * sg_etp_session_open(IUINT32 conv, const struct sockaddr * addr, uv_loop_t * loop, void * data);
int sg_etp_session_set_callback(sg_etp_session_t * session,
    sg_etp_on_open_func_t on_open,
    sg_etp_on_data_func_t on_data,
    sg_etp_on_sent_func_t on_sent,
    sg_etp_on_close_func_t on_close
);
int sg_etp_session_close(sg_etp_session_t * session);
int sg_etp_session_start(sg_etp_session_t * session, int interval_ms, uv_udp_t * udp);
int sg_etp_session_send(sg_etp_session_t * session, const void * data, size_t size);
int sg_etp_session_recv(sg_etp_session_t * session, void * data, size_t size);
void * sg_etp_session_get_data(sg_etp_session_t * session);
char * sg_etp_session_get_client_addr(sg_etp_session_t * session);
IUINT32 sg_etp_session_get_conv(sg_etp_session_t * session);
void sg_etp_session_set_timeout(sg_etp_session_t * session, uint64_t timeout);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_ETP_PRIVATE_H */

