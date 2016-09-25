#include "rudp_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

enum DOWNLOAD_CMD {

	DOWNLOAD_CMD_START = 1, DOWNLOAD_CMD_DATA = 2, DOWNLOAD_CMD_END = 3
};

typedef struct {
	char * remote_path;
	char * local_path;
	FILE * file;
} download_task;

static void start_download(rudp_t *client) {
	//发送一个命令
	download_task *task = client->user_data;
	int size = 1 + strlen(task->remote_path);
	char cmd[size];
	cmd[0] = DOWNLOAD_CMD_START;
	strcpy(cmd + 1, task->remote_path);
	int error = rudp_send(client, cmd, size);
	printf("start download,error=%d\n", error);
}
static void receive_data(rudp_t *client, char *data, size_t size) {
	download_task *task = client->user_data;
	FILE *file = task->file;
	if (!file) {
		file = fopen(task->local_path, "wb");
		if (!file) {
			printf("打开文件失败,%s\n", task->local_path);
			return;
		}
		task->file = file;
	}

	int len = fwrite(data, 1, size, file);
	//printf("接收到的文件数据,%d=%d\n",size,len);

	//要求发送下一个数据
	char nextCmd[] = { DOWNLOAD_CMD_DATA };
	rudp_send(client, nextCmd, 1);
}
static void end_download(rudp_t *client, char *data, size_t size) {
	download_task *task = client->user_data;
	if (task->file) {
		fclose(task->file);
		task->file = NULL;
	}
	if (size > 0) {
		//有错误消息
		printf("下载返回错误:%.*s\n", size, data);
	} else {
		printf("下载成功\n");
	}

	//char nextCmd={DOWNLOAD_CMD_END};
	//rudp_send(client,nextCmd,1);
	//关闭
	rudp_close(client);
}
static void on_open(rudp_t *client) {
	start_download(client);
}

static void on_message(rudp_t *client, char *data, size_t size) {
	char cmd = data[0];
	if (cmd == DOWNLOAD_CMD_DATA) {
		receive_data(client, data + 1, size - 1);
	} else if (cmd == DOWNLOAD_CMD_END) {
		end_download(client, data + 1, size - 1);
	} else {

	}

}
static void on_close(rudp_t *client) {
	printf("on_close\n");
}

static void test_download_file(const char *host, int port,
		const char *remote_path, const char* local_path) {
	rudp_t *client = rudp_open(host, port, on_open, on_message, on_close);
	if (client) {
		//可以设置任何的数据，方便在on_open,on_close,on_message中使用
		download_task *task = malloc(sizeof(download_task));
		memset(task, 0, sizeof(download_task));
		task->remote_path = strdup(remote_path);
		task->local_path = strdup(local_path);
		task->file = NULL;
		client->user_data = task;

		//开始
		rudp_loop(client, 1000);
		//退出后，client就自动释放了，不要再访问

		free(task->remote_path);
		free(task->local_path);
		if (task->file) {
			fclose(task->file);
		}
		free(task);
	}
}

int main(int argc, char **argv) {
	if (argc < 5) {
		printf("client_test <host> <port> <remote_path> <local_path>\n");
		return 0;
	}

	const char *host = argv[1];
	int port = atoi(argv[2]);
	const char *remote_path = argv[3];
	const char *local_path = argv[4];

	//初始化，只需要调用一次
	if (rudp_init() != 0) {
		printf("rudp_init error\n");
		return 0;
	}

	for (int i = 0; i < 1; i++) {
		//可以测试多次
		test_download_file(host, port, remote_path, local_path);
	}

	return 0;

}

