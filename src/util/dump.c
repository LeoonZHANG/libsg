/*
 * dump.c
 * Author: wangwei.
 * Dump toolkit.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../../include/sg/util/dump.h"
#include "../../include/sg/sys/shell.h"

#define DUMP_CMD_LEN_MAX 256

static void sg_dump_shell_callback(enum sg_shell_event evt, const char *line, void *context)
{
    int *core_size = (int *)context;

    if (!core_size)
        return;

    if (evt == SGSHELLEVENT_OVER)
        return;

    if (!line || strlen(line) == 0)
        return;

    if (strcmp(line, "unlimited") == 0)
        *core_size = SG_DUMP_CORE_SIZE_UNLIMITED;
    else
        *core_size = atoi(line);
}

int sg_dump_get_core_size(void)
{
    int core_size = -1;

    sg_shell_exec("ulimit -c", sg_dump_shell_callback, &core_size);

    return core_size;
}

int sg_dump_set_core_size(int size)
{
    char cmd[DUMP_CMD_LEN_MAX];

    if (size >= 0) {
        snprintf(cmd, DUMP_CMD_LEN_MAX, "ulimit -c %d", size);
        return sg_shell_exec2(cmd);
    } else if (size == SG_DUMP_CORE_SIZE_UNLIMITED)
        return sg_shell_exec2("ulimit -c unlimited");
    else
        return -1;
}