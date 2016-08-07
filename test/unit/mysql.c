#include <stdio.h>

#include "mysql.h"

int main()
{
	sg_mysql_con con;
	struct sg_mysql_conn_parm parm;
	char *info = "current ip is forbidden";;
	int times=2, crawl=3, type=2;
	
	parm.host = "10.78.72.200";
	parm.user = "root";
	parm.passwd= "123456";
	parm.db = "crudecut_test";
	parm.port = 3306;

	/* Allocate a new MYSQL object */
	if (sg_mysql_init(&con) != 0)
		return -1;
	
	sg_mysql_set_conn_opts(con);
	/* Connect to MySQL data base*/
	if (sg_mysql_connect(con, parm) != 0)
		if (sg_mysql_reconnect(con, times, parm) != 0) {
			sg_mysql_disconnect(con);
			return -1;
		}
	/* Insert a record into MySQL. */
	if (sg_mysql_insert(con, crawl, type, info)!= 0) {
		sg_mysql_disconnect(con);
		return -1;
	}
	/* Close the connection and free the allocated MYSQL object. */
	sg_mysql_disconnect(con);
	return 0;

}

