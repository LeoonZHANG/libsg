/**
 * Shell.h
 * Bash and command shell IO and interactive APIs.
 */

#ifndef LIBSG_SHELL_H
#define LIBSG_SHELL_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Event for shell callback. */
enum sg_shell_event{
    SGSHELLEVENT_DATA = 0, /* new data */
    SGSHELLEVENT_OVER = 1, /* shell interactive is over */
};

/* Colors for stdout. */
enum sg_shell_color {
    SGSHELLCOLOR_WHITE        = 0, /* white on black */
    SGSHELLCOLOR_CYAN         = 1, /* cyan on black */
    SGSHELLCOLOR_BLUE         = 2, /* blue on black */
    SGSHELLCOLOR_YELLOW       = 3, /* yellow on black */
    SGSHELLCOLOR_RED          = 4, /* read on black */
    SGSHELLCOLOR_WHITE_ON_RED = 5, /* white on read */
};

/* Shell handle. */
typedef struct shell_data sg_shell;

/* Shell callback function type definition.
   line is a string for one line which ends in '\0'. */
typedef void (*sg_shell_callback)
        (enum sg_shell_event evt, const char *line, void *context);

/* Open sync shell.
   Return -1: failure.
   Return 0: succeed. */
int sg_shell_exec(const char *cmd, sg_shell_callback cb, void *context);

/* Open sync shell pipe connection to stdin of child process.
   Output shell exec result string to stdout.
   Return  0: succeed.
   Return -1: failure. */
int sg_shell_exec2(const char *cmd);

/* Open async shell pipe connection to stdout of child process.
   Output shell exec result string by callback.
   Return valid integer: shell handle.
   Return NULL: failure. */
sg_shell *sg_shell_open(const char *cmd, sg_shell_callback cb, void *context);

/* Close async shell. */
void sg_shell_close(sg_shell **s);

/* Colorful stdout. */
void sg_shell_colorful_print(const char *str, enum sg_shell_color color);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_SHELL_H */