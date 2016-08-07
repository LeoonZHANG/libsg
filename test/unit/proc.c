#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "../../libsg/include/proc.h"
#include "../../libsg/include/shell.h"

/* http://lovesoo.org/view-processes-running-linux-full-path-method.html */

#define READ_BUF_SIZE 1024

long *find_pid_by_name(char *pidName)
{
#define PROCESS_LIST_CMD_LEN 256
    int i = 0;
    DIR *dir;
    FILE *fp_status;
    long *pidList = NULL;
    char *line;
    char name[READ_BUF_SIZE];
    char line_buffer[READ_BUF_SIZE];
    char filename[READ_BUF_SIZE];
    struct dirent *next;
    char list_cmd[PROCESS_LIST_CMD_LEN];

    dir = opendir("/proc");
    if (!dir) {
        printf("Cannot open /proc.");
        return NULL;
    }

    while (next = readdir(dir)) {
        printf("dir:%s\n", next->d_name);
        if (strcmp(next->d_name, ".") == 0)
            continue;

        if (strcmp(next->d_name, "..") == 0)
            continue;

        /* If first char isn't a number, we don't want it. */
        if (!isdigit(*next->d_name))
            continue;

        /*sprintf(filename, "/proc/%s/status", next->d_name);
        fp_status = fopen(filename, "r");
        if (!fp_status)
            continue;

        line = fgets(line_buffer, READ_BUF_SIZE - 1, fp_status);
        fclose(fp_status);
        if (!line)
            continue;

        /* Buffer should contain a string like "Name:   binary_name" *//*
        sscanf(line_buffer, "%*s %s", name);
        printf(name);
        printf("\n");*/

        //snprintf(list_cmd, PROCESS_LIST_CMD_LEN, "ls -l /proc/%s", next->d_name);
        //shell_exec(list_cmd, list_proc_shell_callback, NULL);

        /*if (strcmp(name, pidName) == 0) {
            pidList = realloc(pidList, sizeof(long) * (i + 2));
            pidList[i++] = strtol(next->d_name, NULL, 0);
        }*/
    }
    printf("b");

    if (pidList)
        pidList[i] = 0;

    return NULL;
}

void proc_found(const char *id, void *context)
{
    vlstr *filename;
    filename = sg_proc_filename(atoi(id));
    if(vlstrlen(filename) > 0)
        printf("%s", vlstrraw(filename));
}

int main(void)
{
    printf("user id:%d\n", sg_proc_user_id_current());
    sg_proc_list_all(proc_found, NULL);
    return 0;
}