/*
 * proc.h
 * Author: wangwei.
 * process management.
 */

#ifndef LIBSG_PROC_H
#define LIBSG_PROC_H

#include <stdint.h>
#include "os.h"

#if defined(OS_LNX)
# include <sys/types.h> /* pid_t */
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(OS_WIN)
typedef int pid_t;
typedef int uid_t;
#endif

typedef void (*sg_proc_found_callback)(const char *id, void *context);

typedef void (*sg_proc_module_found_callback)(const char *lib, void *context);

enum sg_proc_priviledge {
    SGPROC = 0
};

#define SG_USER_ID_ROOT 0

/* List all process id and return process count or error(-1). */
int sg_proc_id_all(sg_proc_found_callback cb, void *context);

/* Get current process id. */
pid_t sg_proc_id_current(void);

/* Get parent process id of current process. */
pid_t sg_proc_id_parent(void);

/* Get current process id. */
uid_t sg_proc_user_id_current(void);

/* Get process filename by process id.
 * Remember to free return result. */
sg_vlstr_t *sg_proc_filename(pid_t pid);

/* Get process priviledge by process id. */
enum sg_proc_priviledge
      sg_proc_priviledge_get(pid_t pid);

/* Get current process priviledge. */
int sg_proc_privildge_set(enum sg_proc_priviledge privil);

/* Kill process by process id. */
int sg_proc_kill(pid_t pid);

/* Create standalone process from executable binary file path and parameters. */
pid_t sg_proc_open(const char *cmd);

/* List all dependent dynamic libraries of a executable binary file. */
int sg_proc_get_module(const char *exec_filename, sg_proc_module_found_callback cb);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_PROC_H */
