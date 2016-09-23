/**
 * daemon.h
 * Start with OS and independent of terminal.
 */

#ifndef LIBSG_DAEMON_H
#define LIBSG_DAEMON_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Make current process starting with OS. */
int sg_daemon_start_with_os(void);

/* Make current process independent of terminal. */
int sg_daemon_independent_of_terminal(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_DAEMON_H */