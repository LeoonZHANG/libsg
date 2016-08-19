/*
 * sg.c
 * Author: wangwei.
 * Libsg public module.
 */

#include <stdio.h>
#include <string.h>
#include <sg/sys/time.h>

int sg_register(const char *key)
{
    return 0;
}

int sg_check_expiration(void)
{
	time_t today_date;

	today_date = sg_unix_time_s();
	//sg_date_time_s_to_unix_time_s()
	return 0;
}
