/* 
 * etp_server.h
 */

#ifndef LIBSG_ETP_SERVER_H
#define	LIBSG_ETP_SERVER_H

#ifdef	__cplusplus
extern "C"
{
#endif

typedef struct sg_etp_client_real sg_etp_client_t; /*struct sg_etp_client_real的真实定义请放在etp_server.c中*/
typedef struct sg_etp_server_real sg_etp_server_t; /*struct sg_etp_server_real的真实定义请放在etp_server.c中*/

typedef void (*sg_etp_server_on_open_func_t)(sg_etp_client_t *client);
typedef void (*sg_etp_server_on_close_func_t)(sg_etp_client_t *client, int code, const char *reason);
typedef void (*sg_etp_server_on_message_func_t)(sg_etp_client_t *client, char *data, size_t len);

/* 初始化，返回0表示成功，如果不必要，可以撤销此接口 */
int sg_etp_server_init(void);

sg_etp_server_t *sg_etp_server_open(
    const char                     *server_addr, 
    int                             server_port,
    int                             backlog,
    int                             max_conn_count,
    sg_etp_server_on_open_func_t    on_open,
    sg_etp_server_on_message_func_t on_message,
    sg_etp_server_on_close_func_t   on_close
);

int sg_etp_server_send_data(sg_etp_client_t *, void *data, size_t size);

void sg_etp_server_close_client(sg_etp_client_t *);

/* 调用者需要释放返回的char * */
char *sg_etp_server_get_client_addr(sg_etp_client_t *);

/* 实际上就是调用了uv_run之类的接口，让线程保持阻塞，直至etp_server结束时此接口才返回 */
void sg_etp_server_run(sg_etp_server_t *, int interval_ms);

void sg_etp_server_close(sg_etp_server_t *);

/* 如果不必要，可以撤销此接口 */
void sg_etp_server_free(void);

#ifdef	__cplusplus
}
#endif

#endif	/* LIBSG_ETP_SERVER_H */
