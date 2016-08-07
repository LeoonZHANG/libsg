/*
 * @file err.h
 * @author wangwei
 * @brief error subsystem
 */

#ifndef LIBSG_ERR_H
#define LIBSG_ERR_H

#include "../sys/os.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* common errno in libsg */
#define SG_ESTART 20000
#define SG_EOUTOFMEMORY (SG_ESTART + 1)
#define SG_EINVALIDARG  (SG_ESTART + 2)


void sg_err_add(int custom_errno, const char *custom_errmsg);

/* sg_set_errno */
void sg_err_errno_set(int errno);

void sg_err_errno_set_and_log(int errno);

void sg_err_errno_clear(void);

int sg_err_errno_crt(void);

#if defined(OS_WIN)

int sg_err_errno_win(void);

int sg_err_errno_wsa(void);

#endif

const char *sg_err_errmsg_crt(void);

const char *sg_err_errmsg_win(void);

const char *sg_err_errmsg_wsa(void);

const char *sg_err_strerror(int errno);



void sg_set_last_err(int errno);

void sg_clear_last_err(void);

int sg_get_last_err(void);

const char *sg_get_last_err_msg(void);

const char *sg_err_to_msg(int err_num);






const char *sg_err_msg_std(void);

const char *sg_err_msg_win(void);

const char *sg_err_msg_wsa(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_ERR_H */
