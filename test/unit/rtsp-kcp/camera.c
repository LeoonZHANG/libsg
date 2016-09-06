#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include"../../../include/sg/media/rtsp.h"
#include"../../../include/sg/net/etp_server.h"
#include"../../../include/sg/media/player.h"

static sg_player_t *player = NULL;
static void *player_thread(void *);
static char play_filename[1024];
static int mode = 0; /* 0: local_player, 1: rtsp_player, 2: rtsp */
static int etp_server_port;
static char rtsp_server_url[1024];
static int rtsp_thread_count = 0;
FILE *fp_save_rtp = NULL;

static void *rtsp_thread(void *);

static void start_player_thread(void)
{
    int ret;
    pthread_t id;

    /* open rtsp client to fetch data */
    ret = pthread_create(&id, NULL, player_thread, NULL);
    if(ret == 0) {
        printf("create player thread seccess\n");
    } else {
        printf("create player thread error\n");
        exit(-1);
    }
    pthread_join(id, NULL);
}

static void start_rtsp_thread(void *param)
{
    int ret;
    pthread_t id;

    /* check rtsp thread */
    if (rtsp_thread_count > 0) {
        printf("rtsp thread already running\n");
        return;
    } else
        printf("start to open rtsp\n");

    /* open rtsp client to fetch data */
    ret = pthread_create(&id, NULL, rtsp_thread, param);
    if(ret == 0) {
        printf("create rtsp thread seccess\n");
        rtsp_thread_count++;
    } else {
        printf("create rtsp thread error\n");
        exit(-1);
    }
}

static void etp_server_on_open(sg_etp_client_t *client)
{
	int ret;
	pthread_t id;
	char * addr = NULL;

	addr = sg_etp_server_get_client_addr(client);
	printf("conn from %s\n", addr);
	free(addr);
    start_rtsp_thread((void *)client);
}

static void etp_server_on_message(sg_etp_client_t *client, char *data, size_t size)
{
}

void etp_server_on_sent(sg_etp_client_t *client, int status/*0:OK*/, void *data, size_t len)
{
}

static void etp_server_on_error(sg_etp_client_t *client, const char *msg)
{
	printf("etp server error\n");
	exit(-1);
}

static void etp_server_on_close(sg_etp_client_t *client, int code, const char *reason)
{
	printf("etp server closed\n");
}

static void rtsp_on_recv(sg_rtsp_t *rtsp, char *data, size_t size, void *context)
{
	sg_etp_client_t *client = (sg_etp_client_t *)context;

	/* save rtp data */
	/*if (!fp_save_rtp)
		fp_save_rtp = fopen("rtp.mp4", "wb");
	if (fp_save_rtp)
		fwrite(data + 12, size - 12, 1, fp_save_rtp);*/

	/* send rtp data */
	if (mode == 2 && client) {
        sg_etp_server_send(client, data, size);
        printf("send %lu data to client\n", size);
    }

    if (mode == 1 && player) {
        sg_player_put_buf(player, data, size);
        printf("put %lu data to player\n", size);
    }
}

static void *player_thread(void *p)
{
    FILE *fp = NULL;
    char read_buf[2048] = {0};

    player = sg_player_create();
    sg_player_load_buf(player);
    sg_player_play(player);

    if (mode == 0) {
        fp = fopen(play_filename, "rb");
        if (!fp) {
            printf("%s open error\n", play_filename);
            exit(-1);
        }

        while (1) {
            usleep(100);
            size_t s = fread((void *)read_buf, 1, 2048, fp);
            if (s > 0) {
                sg_player_put_buf(player, read_buf, s);
                printf("---------put data %lu\n", s);
            }
        }
    }

    while (1)
        usleep(100);
}

static void *rtsp_thread(void *p)
{
	sg_rtsp_t *r;

	sg_rtsp_init();
	r = sg_rtsp_open(rtsp_server_url, 10000, SGRTSPDATAPROTOCOL_TCP, rtsp_on_recv, NULL, p);
	if (r)
		printf("RTSP connect OK\n");
	else
		printf("RTSP connect error\n");

	sg_rtsp_play(r);
}

int main(int argc,char**argv)
{
	sg_etp_server_t *server;

    if (argc <= 1) {
        printf("输入%d个参数, 错误\n", argc);
        return -1;
    }

    if (strstr(argv[1], "local_player")) {
        if (argc != 3) {
            printf("输入%d个参数, 错误, 需要3个参数\n", argc);
            return -1;
        }
        sprintf(play_filename, "%s", argv[2]);
        mode = 0;
    } else if (strstr(argv[1], "rtsp_player")) {
        if (argc != 3) {
            printf("输入%d个参数, 错误, 需要3个参数\n", argc);
            return -1;
        }
        snprintf(rtsp_server_url, 1024, "%s", argv[2]);
        mode = 1;
    } else if (strstr(argv[1], "rtsp")) {
        if (argc != 4) {
            printf("输入%d个参数, 错误, 需要4个参数\n", argc);
            return -1;
        }
        snprintf(rtsp_server_url, 1024, "%s", argv[2]);
        etp_server_port = atoi(argv[3]);
        mode = 2;
    }

    if (mode == 0) {
        start_player_thread();
        return 0;
    } else if (mode  == 1) {
        start_rtsp_thread(NULL);
        start_player_thread();
        return 0;
    } else {
        /* open etp server to ack data */
        server = sg_etp_server_open("0.0.0.0", etp_server_port, 10,
                                    etp_server_on_open, etp_server_on_message,
                                    etp_server_on_sent, etp_server_on_error, etp_server_on_close);
        if (server)
            printf("etp server open sucess\n");
        else
            printf("etp server open error\n");
        sg_etp_server_run(server, 10);
        return 0;
    }
}