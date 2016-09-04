/**
 * rtsp.h
 * RTSP client based on libcurl.
 * All interfaces are sync.
 */
 
#ifndef LIBSG_RTSP_H
#define LIBSG_RTSP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_rtsp_real sg_rtsp_t;

typedef void (*sg_rtsp_on_recv_func_t)(sg_rtsp_t *, char *data, size_t size, void *context);
typedef void (*sg_rtsp_on_close_func_t)(sg_rtsp_t *, int code, const char *reason, void *context);

/* sync */
int sg_rtsp_init(void);

/* sync */
sg_rtsp_t *sg_rtsp_open(const char *url, unsigned int udp_client_port, bool use_tcp,
                        sg_rtsp_on_recv_func_t, sg_rtsp_on_close_func_t, void *context);

/* sync */
int sg_rtsp_play(sg_rtsp_t *r);

/* sync */
int sg_rtsp_pause(sg_rtsp_t *r);

/*int sg_rtsp_get_speed(sg_rtsp_t *, size_t &send_kbps, size_t &recv_kbps); /*使用speed_stat模块统计速度*/

/* sync */
void sg_rtsp_close(sg_rtsp_t *r);

/* sync */
void sg_rtsp_cleanup(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_RTSP_H */