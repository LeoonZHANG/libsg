#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

//#define PLAY_INSIDE

#include"../../../include/sg/media/rtsp.h"
#include"../../../include/sg/net/etp_server.h"
#ifdef PLAY_INSIDE
#include"../../../include/sg/media/player.h"
#endif

int stream_fd[2] = {0, 0};

#ifdef PLAY_INSIDE
static sg_player_t *player = NULL;
static void *player_thread(void *);
#endif
static char play_filename[1024];
static char read_filename[1024];
static int mode = 0; /* 0: local_player, 1: rtsp_player, 2: rtsp 3: file_etp*/
static int etp_server_port;
static char rtsp_server_url[1024];
static int rtsp_thread_count = 0;
FILE *fp_save_rtp = NULL;

static void *rtsp_thread(void *);
static void *udp_thread(void *);
static void *pipe_thread(void *);

#ifdef PLAY_INSIDE
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
#endif

static void start_udp_thread(void *param)
{
    int ret;
    pthread_t id;

    /* open udp server to wait data */
    ret = pthread_create(&id, NULL, udp_thread, param);
    if(ret == 0) {
        printf("create udp server thread seccess\n");
    } else {
        printf("create udp server thread error\n");
        exit(-1);
    }
}

static void start_pipe_thread(void *param)
{
    int ret;
    pthread_t id;

    /* open udp server to wait data */
    ret = pthread_create(&id, NULL, pipe_thread, param);
    if(ret == 0) {
        printf("create pipe thread seccess\n");
    } else {
        printf("create pipe thread error\n");
        exit(-1);
    }
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

    if (!client)
        return;

	addr = sg_etp_server_get_client_addr(client);
	printf("conn from %s\n", addr);
	free(addr);

    ret = pipe(stream_fd);
    printf("pipe create %s, read %d, write %d\n", ret == 0 ? "success" : "error", stream_fd[0], stream_fd[1]);
    //start_rtsp_thread((void *)client);
    //start_udp_thread((void *)client);
    start_pipe_thread((void *)client);
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

#ifdef PLAY_INSIDE
    if (mode == 1 && player) {
        sg_player_put_buf(player, (char *)data, size);
        printf("put %lu data to player\n", size);
    }
#endif
}

#ifdef PLAY_INSIDE
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
#endif

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

static void *udp_thread(void *p)
{
#define _PORT_ 8071
    sg_etp_client_t *etp_c = (sg_etp_client_t *)p;




    int sock = socket(AF_INET,SOCK_DGRAM,0);
    if(sock < 0)
    {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_port = htons(_PORT_);
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sock,(struct sockaddr*)&local,sizeof(local)) < 0)
    {
        perror("bind");
        exit(1);
    }

    struct sockaddr_in client; //output val
    socklen_t len = sizeof(client); //output val
    char buf[4096];
    memset(buf,'\0',sizeof(buf));
    ssize_t _size;
    while(1)
    {
        _size = recvfrom(sock, buf, 4095,0, (struct sockaddr*)&client, &len);
        if(_size > 0)
        {
            //printf("recv %d data\n", _size);
            if (etp_c) {
                sg_etp_server_send(etp_c, buf, _size);
                //printf("send client %d data\n", _size);
            }
        } else {
            if(_size < 0)
            {
                perror("recvfrom error\n");
                continue;
            }
        }
        //usleep(1);
    }
}

static void *pipe_thread(void *p)
{
    sg_etp_client_t *etp_c = (sg_etp_client_t *)p;

    char buf[4096];
    memset(buf,'\0',sizeof(buf));
    ssize_t _size;
    while(1)
    {
        _size = read(stdin, buf, 4095);
        if(_size > 0)
        {
            printf("pipe read %d data\n", _size);
            if (etp_c) {
                sg_etp_server_send(etp_c, buf, _size);
                printf("send client %d data\n", _size);
            }
        } else {
            if(_size < 0)
            {
                perror("pipe read error\n");
                continue;
            }
        }
        //usleep(1);
    }
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
    } else if (strstr(argv[1], "file_etp")) {
        if (argc != 4) {
            printf("输入%d个参数, 错误, 需要4个参数\n", argc);
            return -1;
        }
        snprintf(read_filename, 1024, "%s", argv[2]);
        etp_server_port = atoi(argv[3]);
        mode = 3;
    }

    if (mode == 0) {
#ifdef PLAY_INSIDE
        start_player_thread();
#endif
        return 0;
    } else if (mode  == 1) {
        start_rtsp_thread(NULL);
#ifdef PLAY_INSIDE
        start_player_thread();
#endif
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
