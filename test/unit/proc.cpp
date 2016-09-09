#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sg/sys/proc.h>
#include <sg/sys/shell.h>
#include <gtest/gtest.h>

void proc_found(const char *id, void *context)
{
#if !defined WIN32
    sg_vlstr_t *filename;
    filename = sg_proc_filename(atoi(id));
    if(sg_vlstrlen(filename) > 0)
        printf("%s", sg_vlstrraw(filename));
#endif
}

TEST(test_sg_proc, user_id_current) {
    uid_t uid = sg_proc_user_id_current();
    ASSERT_GT(uid, 0);
}

TEST(test_sg_proc, id_all) {
    int ret = sg_proc_id_all(proc_found, NULL);
    ASSERT_GE(ret, 0);
}
