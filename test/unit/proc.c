#include <stdio.h>
#include <string.h>
#include <sg/sys/proc.h>
#include <sg/sys/shell.h>


void proc_found(const char *id, void *context)
{
    sg_vlstr *filename;
    filename = sg_proc_filename(atoi(id));
    if(sg_vlstrlen(filename) > 0)
        printf("%s", sg_vlstrraw(filename));
}

int main(void)
{
    printf("user id:%d\n", sg_proc_user_id_current());
    sg_proc_list_all(proc_found, NULL);
    return 0;
}
