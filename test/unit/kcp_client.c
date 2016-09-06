#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "uv.h"
#include "ikcp.h"
#include "etp.h"

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


static char s_path[256] = {0};
static uint32_t now;
static size_t data_size = 0;

static void s_kcp_on_open(sg_etp_t *client)
{
    ftp_t output;

    memset(&output, 0, sizeof(ftp_t));

    printf("start to recv file %s\n", s_path);
    printf("start @ %u\n", now = sg_etp_now(client));

    output.payload = PT_GET;
    strncpy(output.data, s_path, sizeof(output.data));
    output.len = strlen(output.data) + 1;

    sg_etp_send(client, &output, sizeof(ftp_t));
}

static void s_kcp_on_data(sg_etp_t *client, char *data, size_t size)
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
            //printf("%d ", input->offset);
            fseek(fp, input->offset, SEEK_SET);
            fwrite(input->data, 1, input->len, fp);
            fclose(fp);
            data_size += input->len;
            break;
        case PT_BYE:
            printf("transmit finished\n");
            printf("end @ %u, used %u ms, speed: %ld kB/s\n", sg_etp_now(client), sg_etp_now(client) - now, data_size / (sg_etp_now(client) - now));
            output.payload = PT_BYE;
            output.len = 0;
            sg_etp_send(client, &output, sizeof(ftp_t));

            sg_etp_close(client);
            break;
    }
}

static void s_etp_on_sent(sg_etp_t * client, int status/*0:OK*/, void * buf, size_t len)
{}

static void s_kcp_on_close(sg_etp_t *client, int code, const char *reason)
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

    sg_etp_t * client = sg_etp_open(ip, port, s_kcp_on_open, s_kcp_on_data, s_etp_on_sent, s_kcp_on_close);

    sg_etp_run(client, 10);

    /*sg_etp_close(client);*/

    return 0;
}