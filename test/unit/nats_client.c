#include <stdio.h>

#include "sg_nats.h"


void process_msg(sg_nats_msg *nmsg)
{
	sleep(2);
	char *msg;

	msg = (char *)natsMsg_GetData(nmsg);
	printf("%s\n",msg);
}


int main()
{
	int i, times=2;
	int64_t timeout_ms = 3600000;
	sg_nats_con *nc;
	sg_nats_sub *sub;
	const char *url = "nats://172.20.220.156:4222";
	const char *channel = "foo.1";
	const char *msg = "{\"hello\":\"world\"}";

	sg_nats_con_init(&nc);
	sg_nats_sub_init(&sub);
	if (sg_nats_connect(&nc, url) != 0)
		if (sg_nats_reconnect(&nc, times, url) != 0)
			goto EXIT;

	/* a publish instance */
	if (sg_nats_publish(nc, channel, msg) != 0)
		goto EXIT;
	
	/* a subscribe instance */
	sg_nats_subscribe(nc, &sub, channel, timeout_ms, process_msg);
	sg_nats_subscription_destroy(&sub);

		
EXIT:	sg_nats_connect_destroy(&nc);
	sg_nats_close();
	return 0;
}
