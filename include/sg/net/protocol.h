/*
 * protocol.h
 * Author: wangwei.
 * Protocol parser.
 */

#ifndef LIBSG_PROTOCOL_H
#define LIBSG_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum sg_protocol {
	SGPROTOCOL_FTP   = 0,
	SGPROTOCOL_FTPS  = 1,
	SGPROTOCOL_HTTP  = 2,
	SGPROTOCOL_HTTPS = 3, /* https://www.google.com */
    SGPROTOCOL_SMTP  = 4, /* mailto://admin@admin.net */
    SGPROTOCOL_RTSP  = 5,
    SGPROTOCOL_RTMP  = 6
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_PROTOCOL_H */
