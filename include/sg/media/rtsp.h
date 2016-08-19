/*
 * rtsp.h
 * RTSP client based on libcurl.
 */
 
/* https://curl.haxx.se/libcurl/c/rtsp.html */
 
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

sg_rtsp_t *sg_rtsp_open(const char *url,
                        sg_rtsp_on_open_func_t,
                        sg_rtsp_on_data_func_t,
                        sg_rtsp_on_close_func_t);

int sg_rtsp_run(sg_rtsp_t *, int interval_ms);

void sg_rtsp_close(sg_rtsp_t *);

void sg_rtsp_free(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_RTSP_H */