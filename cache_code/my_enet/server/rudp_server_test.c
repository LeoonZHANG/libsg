#include <stdio.h>
#include <sys/stat.h>
#include <enet/enet.h>
#include "rudp_server.h"

enum DOWNLOAD_CMD {

	DOWNLOAD_CMD_START = 1, DOWNLOAD_CMD_DATA = 2, DOWNLOAD_CMD_END = 3
};

static void test_send_file_data(rudp_client_t *client) {
	FILE *file = client->user_data;
	if (!file) {
		return;
	}
	int bulk_size = 64*1024;
	char cmd[1 + bulk_size];
	bulk_size = fread(cmd + 1, 1, bulk_size, file);
	int cmd_size = 0;
	if (bulk_size > 0) {
		cmd[0] = DOWNLOAD_CMD_DATA;
		cmd_size = 1 + bulk_size;
	} else {
		cmd[0] = DOWNLOAD_CMD_END;
		cmd_size = 1;
		fclose(file);
		client->user_data = NULL;
	}
	int error = rudp_server_send_data(client, cmd, cmd_size);
	if(cmd[0]==DOWNLOAD_CMD_DATA){
		printf("发送文件数据,error=%d,size=%d\n", error,bulk_size);
	}else{
		printf("文件发送完毕,error=%d\n", error);
	}


}
static void test_start_download(rudp_client_t *client, const char *data,
		int size) {
	char filename[size + 1];
	strncpy(filename, data, size);
	filename[size]='\0';
	printf("filename=%s\n", filename);
	FILE *file = fopen(filename, "rb");
	if (!file) {
		char error_msg[] = "文件不存在";
		char nextCmd[1 + strlen(error_msg)];
		nextCmd[0] = DOWNLOAD_CMD_END;
		strcpy(nextCmd + 1, error_msg);
		int error = rudp_server_send_data(client, nextCmd, strlen(nextCmd));
		printf("需要下载的文件不存在：%s\n", filename);
		return;
	}

	client->user_data = file;
	test_send_file_data(client);
}

static void on_client_open(rudp_client_t *client) {
	char *ip=rudp_server_get_client_addr(client);
	printf("on_client_open,ip=%s\n",ip);
	free(ip);

}
static void on_client_message(rudp_client_t *client, char *data, size_t size) {

	char cmd = data[0];
	//printf("on_client_message,cmd=%d,size=%d\n", cmd, size);
	if (cmd == DOWNLOAD_CMD_START) {
		test_start_download(client, data + 1, size - 1);
	} else if (cmd == DOWNLOAD_CMD_DATA) {
		test_send_file_data(client);
	}

}
static void on_client_close(rudp_client_t *client) {
	//printf("on_client_close,%d\n", client->user_data);
	FILE *file = client->user_data;
	if (file) {
		fclose(file);
		client->user_data = NULL;
	}

}

static void test(const char *host, int port) {
	rudp_server_t *server = rudp_server_open(host, port, 100, on_client_open,
			on_client_message, on_client_close);
	if (server) {
		//可以设置任何的数据，方便在on_xxx函数调用的时候使用
		//server->user_data=NULL;
		rudp_server_run(server, 1000);
	}

}
int main(int argc, char **argv) {

	if (rudp_init() != 0) {
		printf("rudp_init error\n");
		return 0;
	}

	if (argc < 2) {
		printf("server_test [host] <port>\n");
		return 0;
	}
	char *host = NULL;
	int port = 0;
	if (argc > 2) {
		host = argv[1];
		port = atoi(argv[2]);
	} else {
		host = NULL;
		port = atoi(argv[1]);
	}
	test(host, port);

	return 0;

}

