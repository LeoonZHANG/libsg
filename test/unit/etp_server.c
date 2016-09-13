#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "etp_server.h"

enum
{
    PT_LS,
    PT_GET,
    PT_PUT,
    PT_CHK,
    PT_BYE,
    PT_HELP,
};

typedef struct
{
    int payload;
    size_t offset;
    size_t len;
    char data[1024];
}ftp_t;

static void s_kcp_on_open(sg_etp_client_t * client)
{
    char * addr = NULL;
    /*printf("s_kcp_on_open\n");*/

    addr = sg_etp_server_get_client_addr(client);

    printf("conn from %s, %lu\n", addr, sg_etp_session_get_conv(client));

    free(addr);
}

static void s_kcp_on_data(sg_etp_client_t *client, char *data, size_t size)
{
    ftp_t * input = (ftp_t *)data;
    ftp_t output;
    FILE * fp = NULL;

    switch (input->payload)
    {
        case PT_GET:
            printf("GET %s\n", input->data);
            fp = fopen(input->data, "rb");
            if (NULL == fp)
            {
                printf("open %s failed\n", input->data);
                return;
            }

            output.payload = PT_CHK;
            while ((output.len = fread(output.data, 1, 1024, fp)) > 0)
            {
                output.offset = ftell(fp);
                //printf("%d ", output.offset);
                sg_etp_server_send_data(client, &output, sizeof(ftp_t));
            }
            output.payload = PT_BYE;
            output.len = 0;
            sg_etp_server_send_data(client, &output, sizeof(ftp_t));

            sg_etp_server_close_client(client);

            fclose(fp);
            printf("send over, bye\n");
            break;

        case PT_BYE:
            /*sg_etp_server_close_client(client);*/
            break;
    }
}

void s_kcp_on_sent(sg_etp_client_t *client, int status/*0:OK*/, void *data, size_t len)
{

}


static void s_kcp_on_close(sg_etp_client_t *client, int code, const char *reason)
{
    char * addr = NULL;

    addr = sg_etp_server_get_client_addr(client);

    printf("close conn from %s\n", addr);

    free(addr);
}


int main(int argc, char * argv[])
{
    char sport[16]  = {0};
    int  port;

    if (argc < 2)
    {
        printf("%s port\n", argv[0]);
        return 0;
    }

    if (argc > 1) strncpy(sport, argv[1], sizeof(sport));
    sscanf(sport, "%d", &port);

    printf("listen @ %d\n", port);

    sg_etp_server_t * server = sg_etp_server_open("0.0.0.0", port, 100, s_kcp_on_open, s_kcp_on_data, s_kcp_on_sent, NULL, s_kcp_on_close);

    sg_etp_server_run(server, 10);

    sg_etp_server_close(server);

    return 0;
}

