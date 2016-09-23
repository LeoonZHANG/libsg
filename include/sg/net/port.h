/*
 * port.h
 * Author: wangwei.
 * Default port definition of commonly used services.
 */

/* https://www.douban.com/note/568630865/ */

#ifndef LIBSG_PORT_H
#define LIBSG_PORT_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SG_PORT_DEFAULT_FTP_CTRL   21
#define SG_PORT_DEFAULT_SSH        22
#define SG_PORT_DEFAULT_TELNET     23
#define SG_PORT_DEFAULT_SMTP       25
#define SG_PORT_DEFAULT_WHOIS      43
#define SG_PORT_DEFAULT_DNS        53
#define SG_PORT_DEFAULT_HTTP       80
#define SG_PORT_DEFAULT_POP3       110
#define SG_PORT_DEFAULT_IMAP       143
#define SG_PORT_DEFAULT_HTTPS      443
#define SG_PORT_DEFAULT_RTSP       554
#define SG_PORT_DEFAULT_IMAPS      993    /* IMAP SSL */
#define SG_PORT_DEFAULT_SMTPS      994    /* SMTP SSL */
#define SG_PORT_DEFAULT_POP3S      995    /* POP3 SSL */
#define SG_PORT_DEFAULT_RTMP       1935
#define SG_PORT_DEFAULT_MYSQL      3306
#define SG_PORT_DEFAULT_POSTGRESQL 5432
#define SG_PORT_DEFAULT_REDIS      6379
#define SG_PORT_DEFAULT_MONGODB    27017

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_PORT_H */
