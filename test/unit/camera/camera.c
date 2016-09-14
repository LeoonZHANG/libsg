/**
 * usage example:
 * ./camera file/rtsp your_filename/your_rtsp_url your_etp_server_port
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sg/net/etp_server.h>

#define READBUFSIZE 4096
#define RTSPURLLEN  1024
#define LOCALFILENAMELEN  1024

enum mode {
    MODE_FILE = 0,
    MODE_RTSP = 1
};

static char local_filename[LOCALFILENAMELEN] = {0};
static char rtsp_url[RTSPURLLEN] = {0};
static int  fd[2] = {0, 0};
void start_file_thread(void *param);
static void file_thread_func(void *);
void start_pipe_ffmpeg_thread(void *param);
static void pipe_thread_func(void *);
static void ffmpeg_thread_func(void *);
enum mode mode = MODE_FILE;


static void etp_server_on_open(sg_etp_client_t *client)
{
    int ret;
    pthread_t id;
    char * addr = NULL;

    if (!client)
        return;

    addr = sg_etp_server_get_client_addr(client);
    printf("new client from %s\n", addr);
    free(addr);

    if (mode == MODE_FILE)
        start_file_thread((void *)client);
    else
        start_pipe_ffmpeg_thread((void *)client);
}

static void etp_server_on_recv(sg_etp_client_t *client, char *data, size_t size)
{
    printf("recv %lu bytes data\n", size);
}

void etp_server_on_sent(sg_etp_client_t *client, int status/*0:OK*/, void *data, size_t len)
{
    //printf("sent %lu bytes data\n", len);
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

static void file_thread_func(void *param)
{
    char read_buf[READBUFSIZE] = {0};
    ssize_t read_size;
    int ret;
    FILE *fp_read;
    sg_etp_client_t *client = (sg_etp_client_t *)param;
    char cmd[1024];

    printf("start file thread\n");

    /* open read file */
    fp_read = fopen(local_filename, "rb");
    if (!fp_read) {
        fprintf(stderr, "read file %s open error\n", local_filename);
        goto exit;
    }

    /* loop: read -> send */
    fprintf(stdout, "start to read\n");
    while (1) {
        /* read from file */
        read_size = fread(read_buf, 1, READBUFSIZE, fp_read);
        if (read_size <= 0) {
            fprintf(stderr, "file read end\n");
            break;
        }
        fprintf(stdout, "local file read size %lu", read_size);

        /* send to client */
        if (client)
            sg_etp_server_send(client, read_buf, read_size);
        usleep(1);
    }

    exit:
    printf("file thread exit\n");
    fclose(fp_read);
}

void start_file_thread(void *param)
{
    int ret;
    pthread_t id;

    /* open file thread to read multimedia data */
    ret = pthread_create(&id, NULL, file_thread_func, param);
    if(ret == 0) {
        printf("create file thread seccess\n");
    } else {
        printf("create file thread error\n");
        exit(-1);
    }
}

static void pipe_thread_func(void *param)
{
    char read_buf[READBUFSIZE] = {0};
    ssize_t read_size;
    size_t write_all, write_once;
    int ret;
    FILE *fp_save;
    char cmd[1024];
    sg_etp_client_t *client = (sg_etp_client_t *)param;

    printf("start pipe thread\n");

    /* open save file */
    /*fp_save = fopen("tmp.ts", "wb");
    if (!fp_save) {
        fprintf(stderr, "save file open error\n");
        goto exit;
    }*/

    /* loop: read -> send -> save */
    fprintf(stdout, "start to read\n");
    while (1) {
        /* read from pipe */
        read_size = read(fd[0], read_buf, READBUFSIZE);
        if (read_size <= 0) {
            fprintf(stderr, "pipe read error\n");
            break;
        }
        //printf("read size %lu\n", read_size);

        /* send to client */
        //char *tmp = (char *)malloc(read_size);
        //memcpy(tmp, read_buf, read_size);
        if (client)
            sg_etp_server_send(client, read_buf, read_size);

        /* save to disk */
        /*write_all = 0;
        while (write_all < read_size) {
            write_once = fwrite(&read_buf[write_all], 1, read_size - write_all, fp_save);
            if (write_once == 0) {
                fprintf(stderr, "fwrite error\n");
                exit(-1);
            }
            write_all += write_once;
        }
        usleep(1);*/
    }

    exit:
        printf("pipe thread exit\n");
        close(fd[0]);
        close(fd[1]);
}

static void ffmpeg_thread_func(void *param)
{
    int ret;
    char cmd[1024];

    snprintf(cmd, 1024,
             "ffmpeg -y -rtsp_transport tcp -i %s -vcodec copy -acodec copy -f mpegts pipe:%d",
             rtsp_url, fd[1]);
    fprintf(stdout, "start to run ffmpeg, cmd is:%s\n", cmd);
    ret = system(cmd);
    fprintf(stdout, "ffmpeg exit with %d\n", ret);
}

void start_pipe_ffmpeg_thread(void *param)
{
    int ret;
    pthread_t id_pipe, id_ffmpeg;

    /* open pipe between ffmpeg */
    if (pipe(fd) != 0) {
        fprintf(stderr, "pipe open error\n");
        exit(-1);
    }
    printf("pipe open success, fd[0]:%d, fd[1]:%d\n", fd[0], fd[1]);

    /* open pipe thread to read pipe data */
    ret = pthread_create(&id_pipe, NULL, pipe_thread_func, param);
    if(ret == 0) {
        printf("create pipe thread seccess\n");
    } else {
        printf("create pipe thread error\n");
        exit(-1);
    }

    /* open ffmpeg thread to write multimedia data to pipe */
    ret = pthread_create(&id_ffmpeg, NULL, ffmpeg_thread_func, NULL);
    if(ret == 0) {
        printf("create ffmpeg thread seccess\n");
    } else {
        printf("create ffmpeg thread error\n");
        exit(-1);
    }
}

int main(int argc, char**argv)
{
    int etp_server_port = 0;
    sg_etp_server_t *server;

    /* parse param */
    if (argc < 4) {
        printf("需要4个参数, 实际输入输入%d个参数\n", argc);
        return -1;
    }
    if (strstr(argv[1], "file"))
        mode = MODE_FILE;
    else if (strstr(argv[1], "rtsp"))
        mode = MODE_RTSP;
    else {
        printf("unknown read mode:%s\n", argv[1]);
        exit(-1);
    }
    if (mode == MODE_FILE)
        snprintf(local_filename, LOCALFILENAMELEN, "%s", argv[2]);
    else
        snprintf(rtsp_url, RTSPURLLEN, "%s", argv[2]);
    etp_server_port = atoi(argv[3]);
    if (mode == MODE_FILE)
        printf("read file:%s\n", local_filename);
    else
        printf("rtsp url:%s\n", rtsp_url);
    printf("etp server listen port:%d\n", etp_server_port);

    /* open etp server to ack data */
    server = sg_etp_server_open("0.0.0.0", etp_server_port, 10,
                                etp_server_on_open, etp_server_on_recv,
                                etp_server_on_sent, etp_server_on_error, etp_server_on_close);
    if (server)
        printf("etp server open success\n");
    else
        printf("etp server open error\n");
    sg_etp_server_run(server, 10);
    return 0;
}