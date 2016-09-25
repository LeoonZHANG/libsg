/**
 * daemon.h
 * Start with OS and independent of terminal.
 * 注意,不同的linux发行版如何让普通程序随即启动的方法可能有差异,需要兼容
 */

#ifndef LIBSG_DAEMON_H
#define LIBSG_DAEMON_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Make current process starting with OS. */
int sg_daemon_start_with_os(bool on);

/* Make current process independent of terminal. */
int sg_daemon_independent_of_terminal(bool on);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_DAEMON_H */