/**
 * http_client.c
 * Test for libsg module http client.
 */

#include <stdio.h>
#include <string.h>
#include "../../include/http_client.h"
#include "../../include/log.h"

static void http_callback(struct sg_http_client_reply *reply, void *user_data)
{
    printf("http return status:%d, data:%s.", reply->status_code, (char *)reply->content);
}

int main(void)
{
    int ret;

    if (sg_http_client_init() != 0)
        return -1;

    sg_http_client_t *c = sg_http_client_create();
    ret = sg_http_client_set_base(c, "http://38.121.61.254:8888/docs/local/TwitterData/2e21ebec4e544faa439ee3a92bddd9e7", "GET");
    /*sg_http_client_set_post_fields(c, "{\"key\":\"value\"}");*/
    sg_http_client_set_recv_callback(c, http_callback, NULL);

    sg_http_client_run(c);
    sg_http_client_destroy(c);
    return 0;
}
