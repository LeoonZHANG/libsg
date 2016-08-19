/**
 * etp.h
 * ETP (extreme transfer protocol) is a transfer protocol based on LIBUV / UDP / KCP and FEC.
 * This is etp client.
 */
 
#ifndef LIBSG_ETP_H
#define LIBSG_ETP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_etp_real sg_etp_t;

typedef void (*sg_etp_on_open_func_t)(sg_etp_t *);
typedef void (*sg_etp_on_data_func_t)(sg_etp_t *, char *data, size_t size);
typedef void (*sg_etp_on_sent_func_t)(sg_etp_t *, int status/*0:OK*/);
typedef void (*sg_etp_on_close_func_t)(sg_etp_t *, int code, const char *reason);

int sg_etp_init();

sg_etp_t *sg_etp_open(const char *server_addr, int server_port,
                     sg_etp_on_open_func_t,
                     sg_etp_on_data_func_t,
                     sg_etp_on_sent_func_t,
                     sg_etp_on_close_func_t);

int sg_etp_run(sg_etp_t *, int interval_ms);

int sg_etp_send(sg_etp_t *, const void *data, uint64_t size);

uint32_t sg_etp_now(sg_etp_t *);

/* 限制客户端发送速度, kbps为0不做任何限制 */
void sg_etp_set_max_send_speed(sg_etp_t *, size_t kbps);

int sg_etp_get_speed(sg_etp_t *, size_t &send_kbps, size_t &recv_kbps);

void sg_etp_close(sg_etp_t *);

void sg_etp_free(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_ETP_H */