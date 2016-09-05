#include <stdio.h>
#include <stdlib.h>
#include <sg/net/etp.h>
#include <sg/media/player.h>

static sg_player_t *player;

static void etp_on_open(sg_etp_t *client)
{
	player = sg_player_create();
	sg_player_load_buf(player);
	sg_player_play(player);
}


static void etp_on_message(sg_etp_t *client, char *data, size_t size)
{
	int ret = player_put_buf(player, data, size);
	if (ret != size)
		printf("player put buffer error\n");
}

void etp_on_sent(sg_etp_t *client, int status/*0:OK*/)
{
}

static void etp_on_close(sg_etp_t *client, int code, const char *reason)
{
	player_destroy(player);
    printf("conn closed\n");
}

int main(int argc, char const *argv[])
{
	char server_ip[256]={0};
	int server_port;
	char save_path[256]={0};
	sg_etp_t *client;

	if(argc < 4) {
		printf("%s server_ip server_port save_path\n", argv[1]);
		return 0;
	}

	if (argc > 1) sprintf(server_ip, "%s", argv[1]);
    if (argc > 2) server_port = atoi(argv[2]);
    if (argc > 3) sprintf(save_path, "%s", argv[3]);

    client = sg_etp_open(server_ip, server_port, etp_on_open, etp_on_message, etp_on_sent, etp_on_close);
    sg_etp_run(client, 10);
    return 0;
}
