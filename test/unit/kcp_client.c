#include <unistd.h>
#include <stdio.h>
#include "kcp.h"

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
    int len;
    char data[1024];
}ftp_t;


static char s_path[256] = {0};
static uint32_t now;

static void s_kcp_on_open(sg_kcp_t *client)
{
    ftp_t output;

    printf("start to recv file %s\n", s_path);
    printf("start @ %u\n", now = sg_kcp_now(client));

    output.payload = PT_GET;
    strncpy(output.data, s_path, sizeof(output.data));
    output.len = strlen(output.data) + 1;

    sg_kcp_send(client, &output, output.len + 2*sizeof(int));
}

static void s_kcp_on_message(sg_kcp_t *client, char *data, size_t size)
{
    ftp_t * input = (ftp_t *)data;
    ftp_t output;
    FILE * fp;

    switch (input->payload)
    {
        case PT_CHK:
            fp = fopen("data1.dat", "ab");
            if (NULL == fp)
            {
                printf("open %s failed\n", s_path);
                return;
            }
            fwrite(input->data, 1, input->len, fp);
            fclose(fp);
            break;
        case PT_BYE:
            printf("transfer finished\n");
            printf("end @ %u, used %u ms\n", sg_kcp_now(client), sg_kcp_now(client) - now);
            output.payload = PT_BYE;
            output.len = 0;
            sg_kcp_send(client, &output, output.len + 2*sizeof(int));
            
            sg_kcp_close(client);
            break;
    }
}

static void s_kcp_on_close(sg_kcp_t *client, int code, const char *reason)
{
    printf("conn closed\n");
}

int main(int argc, char * argv[])
{
    char ip[32]     = {0};
    char sport[16]  = {0};
    char path[256]  = {0};
    int  port;
	
	if (argc < 4)
	{
		printf("%s host port file_path\n", argv[0]);
		return 0;
	}

    if (argc > 1) strncpy(ip,    argv[1], sizeof(ip));
    if (argc > 2) strncpy(sport, argv[2], sizeof(sport));
    if (argc > 3) strncpy(path,  argv[3], sizeof(path));

    sscanf(sport, "%d", &port);

    printf("  ip: %s\n",   ip);
    printf("port: %d\n", port);
    printf("file: %s\n", path);

    strncpy(s_path, path, sizeof(s_path));

    sg_kcp_t * client = sg_kcp_open(ip, port, s_kcp_on_open, s_kcp_on_message, s_kcp_on_close);

    sg_kcp_loop(client, 10);

    /*sg_kcp_close(client);*/

    return 0;
}