/*
 * rtsp.h
 * RTSP client based on libcurl.
 */
 
/* 参考：
 * https://curl.haxx.se/libcurl/c/rtsp.html
 * 上述demo是基于easy系列接口写的，多线程可能有问题，请改为multi系列接口的实现*/
 
#ifndef LIBSG_RTSP_H
#define LIBSG_RTSP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_rtsp_real sg_rtsp_t;

typedef void (*sg_rtsp_on_open_func_t)(sg_rtsp_t *);
typedef void (*sg_rtsp_on_data_func_t)(sg_rtsp_t *, char *data, size_t size);
typedef void (*sg_rtsp_on_close_func_t)(sg_rtsp_t *, int code, const char *reason);

int sg_rtsp_init();

sg_rtsp_t *sg_rtsp_open_url(const char *url,
                        sg_rtsp_on_open_func_t,
                        sg_rtsp_on_data_func_t,
                        sg_rtsp_on_close_func_t);
                        
sg_rtsp_t *sg_rtsp_open_sdp(const char *url,
                        sg_rtsp_on_open_func_t,
                        sg_rtsp_on_data_func_t,
                        sg_rtsp_on_close_func_t);

int sg_rtsp_run(sg_rtsp_t *, int interval_ms);

/*int sg_rtsp_get_speed(sg_rtsp_t *, size_t &send_kbps, size_t &recv_kbps); /*使用speed_stat模块统计速度*/

void sg_rtsp_close(sg_rtsp_t *);

void sg_rtsp_free(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_RTSP_H */