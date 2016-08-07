#include <stdio.h>  
#include <stdlib.h>
#include <string.h> 

#include "redis.h"


int main()  
{  
	int times = 2, port=6379;
	long sec=2, usec=500000;											
	sg_redis_ctx *ctx;
	struct sg_redis_reply rep;
	char *ip = "172.20.220.156";
	char *value = "hello\\0world\"this is redis\"";				/* data is about to be pushed */
	char list_name[] = "hash";

	/* try to reconnect once fail to connect at first time */
	if (sg_redis_connect(&ctx, ip, port, sec, usec) != 0)			
		if (sg_redis_reconnect(&ctx, times, ip, port, sec, usec) != 0)
			return -1;	

	/* REDIS_ERROR indicates the context can't be reused,you have to set up a new one */
	sg_redis_list_rpush(ctx, &rep, list_name, value);
	switch(rep.type)
	{
	case SGREPLYTYPE_OK:
		printf("Succeed:%d\n",rep.integer);
		break;
	case SGREPLYTYPE_ERROR:
		break;
	case SGCONNECTIONTYPE_ERROR:
		/*
		In this case, must reconnect to Redis again, and then do operations you want here
		*/
		break;
	};
	sg_redis_reply_free(&rep); /* free MyRedisReply object in order to reuse it next */

	sg_redis_list_lpop(ctx, &rep, list_name);
	switch(rep.type)
	{
	case SGREPLYTYPE_OK:
		printf("Succeed to extract data: %s\n",rep.data);
		break;
	case SGREPLYTYPE_EMPTY:
		printf("No data in the list\n");
		break;
	case SGREPLYTYPE_ERROR:
		break;
	case SGCONNECTIONTYPE_ERROR:
		/*
		In this case, must reconnect to Redis again, and then do operations you want here
		*/
		break;
	};	
	sg_redis_reply_free(&rep);
	sg_redis_disconnect(ctx);
	return 0;
}
