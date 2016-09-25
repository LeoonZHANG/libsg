/**
 * signal.h
 * 直接调用https://github.com/apache/apr/blob/trunk/include/apr_signal.h,基本一一对应
 */

#ifndef LIBSG_SIGNAL_H
#define LIBSG_SIGNAL_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef void (*sg_signal_cb_t)(int signum);




void sg_signal_register(int signum, sg_signal_cb_t cb);

const char *sg_signal_get_desc(int signum);

bool sg_signal_block(int signum);

bool sg_signal_unblock(int signum);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_SIGNAL_H */