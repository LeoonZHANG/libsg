#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include"../../../include/sg/media/rtsp.h"
#include"../../../include/sg/net/etp_server.h"

#define PLAY_INSIDE

#ifdef PLAY_INSIDE
#include"../../../include/sg/media/player.h"
static sg_player_t *player = NULL;
#endif

static int etp_server_port;
static char rtsp_server_url[1024];
static int rtsp_thread_count = 0;
FILE *fp_save_rtp = NULL;

static void *rtsp_thread(void *);
static void *player_thread(void *);

static void start_rtsp_thread(void)
{
	int ret;
	pthread_t id;

	/* check rtsp thread */
	if (rtsp_thread_count > 0) {
		printf("rtsp thread running\n");
		return;
	} else
		printf("start to open rtsp\n");

	/* open rtsp client to fetch data */
	ret = pthread_create(&id, NULL, rtsp_thread, NULL);
	if(ret == 0) {
		printf("\n\ncreate thread seccess, and listen @ %d\n\n", etp_server_port);
		rtsp_thread_count++;
	} else {
		printf("\n\ncreate thread error, and listen @ %d\n\n", etp_server_port);
		exit(-1);
	}
}

static void start_player_thread(void)
{
#ifdef PLAY_INSIDE
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
#endif
}

static void etp_server_on_open(sg_etp_client_t *client)
{
	int ret;
	pthread_t id;
	char * addr = NULL;
	addr = sg_etp_server_get_client_addr(client);
	printf("conn from %s\n", addr);
	free(addr);
#if 0
	/* check rtsp thread */
	if (rtsp_thread_count > 0) {
		printf("rtsp thread running\n");
		return;
	} else
		printf("start to open rtsp\n");

	/* open rtsp client to fetch data */
	ret = pthread_create(&id, NULL, rtsp_thread, client);
	if(ret == 0) {
		printf("\n\ncreate thread success, and listen @ %d\n\n", etp_server_port);
		rtsp_thread_count++;
	} else {
		printf("\n\ncreate thread error, and listen @ %d\n\n", etp_server_port);
		exit(-1);
	}
#endif
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
	if (client) {
        sg_etp_server_send(client, data, size);
        printf("send %lu data to client\n", size);
    }

#ifdef PLAY_INSIDE
    if (player)
        sg_player_put_buf(player, data, size);
#endif
}

static void *player_thread(void *p)
{
#ifdef PLAY_INSIDE
    player = sg_player_create();
    sg_player_load_buf(player);
    sg_player_play(player);
#endif
}

static void *rtsp_thread(void *p)
{
	sg_rtsp_t *r;

	sg_rtsp_init();
	r = sg_rtsp_open(rtsp_server_url, 10000, SGRTSPDATAPROTOCOL_TCP, rtsp_on_recv, NULL, NULL);
	if (r)
		printf("RTSP connect OK\n");
	else
		printf("RTSP connect error\n");

	sg_rtsp_play(r);
}

int main(int argc,char**argv)
{
	sg_etp_server_t *server;

	if(argc == 3){
		snprintf(rtsp_server_url, 1024, "%s", argv[1]);
		etp_server_port = atoi(argv[2]);
	} else{
		printf("输入有误!");
		return -1;
	}

    start_player_thread();

	/* open etp server to ack data */
	server = sg_etp_server_open("0.0.0.0", etp_server_port, 10,
								etp_server_on_open, etp_server_on_message,
								etp_server_on_sent, etp_server_on_error, etp_server_on_close);
	if (server)
		printf("etp server open sucess\n");
	else
		printf("etp server open error\n");
	start_rtsp_thread();
	sg_etp_server_run(server, 10);
}