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
	SG_PROTOCOL_FTP   = 0,
	SG_PROTOCOL_FTPS  = 1,
	SG_PROTOCOL_HTTP  = 2,
	SG_PROTOCOL_HTTPS = 3,
}

/*

HTTP
RTSP
RTMP
POP3
mailto://

*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_PROTOCOL_H */
